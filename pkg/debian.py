#!/bin/python

from pathlib import Path
from shutil import copyfile, copytree, rmtree
from os import system, chmod, remove, rmdir
import stat
import argparse

parser = argparse.ArgumentParser(description='Build .deb package after compiling nancho.')
parser.add_argument('--clean', action='store_true')

target_dir = "debian_package"
package_name = "nancho-0.0.2.deb"

directory_tree = [
    "usr/local/bin",
    "etc/systemd/user"
]

targets = [
    ("../build/src/nancho", "usr/local/bin/nancho"),
    ("../resources/nancho.service", "etc/systemd/user/nancho.service")
]

def main():
    args = parser.parse_args()

    if args.clean == True:
        try:
            rmtree(target_dir)
        except:
            pass

        try:
            remove(package_name)
        except:
            pass
    else:

        copytree("DEBIAN", f'{target_dir}/DEBIAN')

        for source, target in targets:
            path = Path(f'{target_dir}/{target}')
            path.parent.mkdir(parents=True, exist_ok=True)
            copyfile(source, f'{target_dir}/{target}')

        chmod(f"{target_dir}/usr/local/bin/nancho", stat.S_IRWXU | stat.S_IRGRP | stat.S_IXGRP | stat.S_IROTH)
        system(f'dpkg-deb --build ./{target_dir} {package_name}')

if __name__ == "__main__":
    main()