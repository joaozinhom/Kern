{
  description = "Kern development shell";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    esp-dev.url = "github:mirrexagon/nixpkgs-esp-dev";
  };

  outputs = { self, nixpkgs, esp-dev }:
    let
      system = "x86_64-linux";

      pkgs = import nixpkgs { inherit system; };

      espPkgs = import esp-dev.inputs.nixpkgs {
        inherit system;
        overlays = [ esp-dev.overlays.default ];
        config.permittedInsecurePackages = [
          "python3.12-ecdsa-0.19.1"
          "python3.13-ecdsa-0.19.1"
        ];
      };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        name = "esp-idf-shell";

        buildInputs = [
          pkgs.gcc
          pkgs.gnumake
          pkgs.just
          espPkgs.esp-idf-full
        ];

        shellHook = ''
          echo "Kern dev-shell ready"
          echo "  gcc:        $(gcc --version | head -1)"
          echo "  make:       $(make --version | head -1)"
          echo "  just:       $(just --version)"
        '';
      };
    };
}