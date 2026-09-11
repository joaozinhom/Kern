from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


class M5StickSupportTests(unittest.TestCase):
    def test_both_m5stick_boards_are_selectable(self):
        kconfig = (ROOT / "main/Kconfig.projbuild").read_text()
        self.assertIn("config KERN_BOARD_M5STICKC_PLUS", kconfig)
        self.assertIn("config KERN_BOARD_M5STICKC_PLUS2", kconfig)

    def test_each_board_has_an_esp32_defaults_file(self):
        for board in ("m5stickc_plus", "m5stickc_plus2"):
            defaults = (ROOT / f"sdkconfig.defaults.{board}").read_text()
            self.assertIn('CONFIG_IDF_TARGET="esp32"', defaults)
            self.assertIn(f"CONFIG_KERN_BOARD_{board.upper()}=y", defaults)

    def test_m5stick_bsp_is_selected_and_camera_sources_are_replaced(self):
        cmake = (ROOT / "main/CMakeLists.txt").read_text()
        self.assertIn("CONFIG_KERN_BOARD_M5STICKC_PLUS", cmake)
        self.assertIn("m5stickcplus", cmake)
        self.assertTrue((ROOT / "main/qr/scanner_no_camera.c").is_file())
        self.assertTrue((ROOT / "main/pages/capture_entropy_no_camera.c").is_file())

    def test_m5stick_bsp_defines_both_hardware_variants(self):
        board_header = (
            ROOT
            / "components/m5stickcplus/include/bsp/m5stickcplus.h"
        ).read_text()
        self.assertIn("CONFIG_KERN_BOARD_M5STICKC_PLUS", board_header)
        self.assertIn("CONFIG_KERN_BOARD_M5STICKC_PLUS2", board_header)
        self.assertIn("BSP_LCD_H_RES", board_header)
        self.assertIn("BSP_BUTTON_A", board_header)


if __name__ == "__main__":
    unittest.main()
