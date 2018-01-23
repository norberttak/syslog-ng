#/bin/env python

import subprocess, re, sys

secret_pattern = b'macska'

# (dolist (char (string-to-list "egy aprocska kalapocska, benne minden csacska macska mocska"))
#  (insert (+ 1 char)))

subprocess.call(["gdb", "meant_to_be_crashed",
                 "-batch",
                 "-ex", "b break_point_for_gdb",
                 "-ex", "r fhz!bqspdtlb!lbmbqpdtlb-!cfoof!njoefo!dtbdtlb!nbdtlb!npdtlb",
                 "-ex", "generate-core-file core-with-potential-secrets"])

with open("core-with-potential-secrets", "rb") as f:
    content = f.read()
    regex = re.compile(secret_pattern)
    result = regex.search(content)

    if result:
        sys.stderr.write("Secret found in core file {}\n".format(secret_pattern))
        sys.exit(1)
    else:
        sys.stderr.write("Test ok\n")
        sys.exit(0)
