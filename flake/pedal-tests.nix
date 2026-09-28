{lib, ...}: {
  perSystem = {pkgs, ...}: {
    # Host-side tests for the RP2040 pedal/IR gesture code (pedals/tests).
    checks.pedal-gesture-tests =
      pkgs.runCommandCC "pedal-gesture-tests" {
        src = lib.fileset.toSource {
          root = ../pedals;
          fileset = ../pedals;
        };
      } ''
        cp -r --no-preserve=mode "$src" pedals
        bash pedals/tests/run.sh
        touch "$out"
      '';
  };
}
