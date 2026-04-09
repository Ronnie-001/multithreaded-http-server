{
  description = "C++ Multithreaded HTTP Server project.";
    
  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
  in {
    devShells.${system}.default = pkgs.mkShell {
      # nativeBuildInputs is for development tools
      nativeBuildInputs = with pkgs; [
        gnumake
        bear          # Generates compile_commands.json for clangd
        clang-tools   # Your LSP
        cmake
      ];

      # mkShell automatically pulls in the C/C++ standard libraries (stdenv)
      # so <iostream> and <sys/socket.h> will instantly become available!
    
    # This runs every time you enter the shell
    shellHook = ''
      echo "C++ Dev Environment Loaded!"
      echo "Node version: $(gcc --version)"
      '';
    };
  };
}
