{lib, ...}: {
  perSystem = {pkgs, ...}: {
    packages = lib.optionalAttrs pkgs.stdenvNoCC.hostPlatform.isDarwin {
      macos-key-logger = pkgs.writers.writePython3Bin "macos-key-logger" {
        libraries = [pkgs.python3Packages.pyobjc-framework-Quartz];
        doCheck = false;
      } (builtins.readFile ../scripts/macos-key-logger.py);
    };
  };
}
