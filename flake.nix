{
    description = "C Application Engine";

    inputs.nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

    outputs = { self, nixpkgs }:
        let
            systems = ["x86_64-linux"];
            forAllSystems = f: builtins.listToAttrs (map (system: {
                name = system;
                value = f system;
            }) systems);
        in {
            devShells = forAllSystems (system : 
            let 
                pkgs = nixpkgs.legacyPackages.${system};
            in {
                default = pkgs.mkShell rec {
                    packages = with pkgs; [
                        renderdoc
                        libX11.dev
                        vulkan-tools
                        vulkan-headers
                        vulkan-loader
                        vulkan-validation-layers
                        vulkan-tools-lunarg
                        shaderc
                    ];

                    shellHook = ''
                        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${toString (pkgs.lib.makeLibraryPath packages)}";
                        nvim .
                        '';
                    };
            });

        };
}
