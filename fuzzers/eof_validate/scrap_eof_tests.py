#!/usr/bin/python
import argparse
import os
import json

cli = argparse.ArgumentParser(
    prog='scrap_eof_tests',
    description='''
    Browses the JSON EOF validation tests and outputs all found input vectors to fuzzing corpus''',
)
cli.add_argument('eof_tests_dir', help="input directory with EOF tests")
cli.add_argument('out_dir', help="corpus output directory")
cli.add_argument('--prefix', required=True, help="output file name prefix")
args = cli.parse_args()


def test_name_to_file_name(name):
    name = name.replace('tests/prague/eip7692_eof_v1/', '')
    name = name.replace('/', '-')
    name = name.replace('::', '-')
    name = name[-160:]
    return name


def collect_vectors(json_test):
    collection = {}
    for test_name, test in json_test.items():
        out_file_name = test_name_to_file_name(test_name)
        if test['vectors'] is None:
            continue  # Some generated tests may have no vectors
        for vector_name, vector in test['vectors'].items():
            bytecode = bytes.fromhex(vector['code'].removeprefix('0x'))
            collection[args.prefix + '-' + out_file_name + '-' + vector_name] = bytecode
    return collection


for root, dirs, files in os.walk(args.eof_tests_dir):
    collection = {}
    for file in files:
        if file.endswith('.json'):
            with open(root + '/' + file) as f:
                json_test = json.load(f)
                collection |= collect_vectors(json_test)
    for file_name, bytecode in collection.items():
        with open(args.out_dir + '/' + file_name, 'wb') as f:
            f.write(bytecode)
