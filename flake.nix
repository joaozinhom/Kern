{
  description = "Kern development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs = { self, nixpkgs, esp-dev }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = nixpkgs.lib.genAttrs systems;
    in
    {
      devShells = forAllSystems (system:
        let
          pkgs = import nixpkgs { inherit system; };

          espPkgs = import esp-dev.inputs.nixpkgs {
            inherit system;
            overlays = [ esp-dev.overlays.default ];
            config.permittedInsecurePackages = [
              "python3.12-ecdsa-0.19.1"
              "python3.13-ecdsa-0.19.1"
            ];
          };

          # Build the shell with clang everywhere: it is the native toolchain
          # on Darwin, and it keeps the host compiler the same on both
          # platforms so .clang-tidy / .clangd see one set of builtin headers.
          mkShell = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; };
        in
        {
          default = mkShell {
            name = "esp-idf-shell";

            buildInputs = [
              pkgs.clang
              # clang-format, clang-tidy and clangd, for scripts/format.sh
              # and the checked-in .clang-tidy / .clangd configs.
              pkgs.clang-tools
              pkgs.gnumake
              pkgs.just
              # nproc, used by the simulator recipes, is GNU-only.
              pkgs.coreutils
              # NOTE: nixpkgs-esp-dev only packages IDF v5.5.x. This project
              # targets v6.x (CI uses espressif/idf:v6.0.2), so idf.py from
              # this shell cannot build the firmware yet.
              espPkgs.esp-idf-full
            ];

            shellHook = ''
              echo "Kern dev-shell ready"
              echo "  clang:      $(clang --version | head -1)"
              echo "  format:     $(clang-format --version)"
              echo "  make:       $(make --version | head -1)"
              echo "  just:       $(just --version)"
              echo "  IDF_PATH:   $IDF_PATH"
            '';
          };
        });
    };
}