#!/bin/sh

echo "[global]"
echo "argc: $#"
echo "argv[0]: $0"
echo "argv[1]: $1"
echo "argv[2]: $2"
echo "argv: $@"
echo "argv: $*"

fn()
{
	echo "argc: $#"
	echo "argv[0]: $0"
	echo "argv[1]: $1"
	echo "argv[2]: $2"
	echo "argv: $@"
	echo "argv: $*"
}

echo "[function]"
fn a b

print_args()
{
	echo "$#: $@"
}

echo "@: $@"
print_args "0$@1"
print_args 0$@1
echo "*: $*"
print_args "0$*1"
print_args 0$*1
