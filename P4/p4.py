#! /bin/env python

import toolspath
from testing import Xv6Build, Xv6Test


class test1(Xv6Test):
    name = "test_1"
    description = "Checks the presence of all system calls"
    tester = "1_starter_tests/test_1.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test2(Xv6Test):
    name = "test_2"
    description = "Validates the output of `getpgdirinfo()` before placing any mmap."
    tester = "1_starter_tests/test_2.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test3(Xv6Test):
    name = "test_3"
    description = "Validates the output of `getwmapinfo()` before placing any mmap."
    tester = "1_starter_tests/test_3.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test4(Xv6Test):
    name = "test_4"
    description = "Places one anonymous map at a fixed address."
    tester = "2_wmap_tests/test_4.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test5(Xv6Test):
    name = "test_5"
    description = "Places one anonymous map at a NOT-fixed address."
    tester = "2_wmap_tests/test_5.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test6(Xv6Test):
    name = "test_6"
    description = "Places one filebacked map at a fixed address."
    tester = "2_wmap_tests/test_6.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test7(Xv6Test):
    name = "test_7"
    description = "Places multiple maps at different addresses."
    tester = "2_wmap_tests/test_7.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test8(Xv6Test):
    name = "test_8"
    description = (
        "Places a large number of anonymous maps at fixed and non-fixed addresses"
    )
    tester = "2_wmap_tests/test_8.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test9(Xv6Test):
    name = "test_9"
    description = "access anonymous map (checks for memory allocation)"
    tester = "2_wmap_tests/test_9.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test10(Xv6Test):
    name = "test_10"
    description = "access one big filebacked map (checks for memory allocation)"
    tester = "2_wmap_tests/test_10.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test11(Xv6Test):
    name = "test_11"
    description = "Places a large number of anonymous maps at fixed and non-fixed addresses and then accesses all pages of each map."
    tester = "2_wmap_tests/test_11.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test12(Xv6Test):
    name = "test_12"
    description = "Checks for lazy allocation in fixed anon mapping"
    tester = "2_wmap_tests/test_12.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test13(Xv6Test):
    name = "test_13"
    description = "Checks for lazy allocation in fixed filebacked mapping"
    tester = "2_wmap_tests/test_13.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test14(Xv6Test):
    name = "test_14"
    description = "Places one anonymous map at a fixed address and unmaps it."
    tester = "3_wunmap_tests/test_14.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test15(Xv6Test):
    name = "test_15"
    description = "Places one anonymous map at a NOT-fixed address and unmaps it."
    tester = "3_wunmap_tests/test_15.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test16(Xv6Test):
    name = "test_16"
    description = "Places one filebacked map at a fixed address and unmaps it."
    tester = "3_wunmap_tests/test_16.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test17(Xv6Test):
    name = "test_17"
    description = "access anonymous map and unmap it"
    tester = "3_wunmap_tests/test_17.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test18(Xv6Test):
    name = "test_18"
    description = "access one big filebacked map (checks for memory allocation)"
    tester = "3_wunmap_tests/test_18.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test19(Xv6Test):
    name = "test_19"
    description = "change one small private filebacked map (checks for memory allocation)"
    tester = "3_wunmap_tests/test_19.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test20(Xv6Test):
    name = "test_20"
    description = "change one big shared filebacked map (checks for memory allocation)"
    tester = "3_wunmap_tests/test_20.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test21(Xv6Test):
    name = "test_21"
    description = "Validates mmaps exists in the mmap list in both parent and child for anonymous private and shared mappings with fork"
    tester = "4_fork_tests/test_21.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test22(Xv6Test):
    name = "test_22"
    description = "Validates parent can NOT see child's change for anonymous private mappings with fork"
    tester = "4_fork_tests/test_22.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test23(Xv6Test):
    name = "test_23"
    description = "Validates parent and child can NOT see eachothers change for anonymous private mappings with fork"
    tester = "4_fork_tests/test_23.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test24(Xv6Test):
    name = "test_24"
    description = "Validates parent can NOT see child's change for filebacked private mappings with fork"
    tester = "4_fork_tests/test_24.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test25(Xv6Test):
    name = "test_25"
    description = "Validates parent can see child's change for anonymous shared mappings with fork"
    tester = "4_fork_tests/test_25.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test26(Xv6Test):
    name = "test_26"
    description = "Validates parent and child can see each others changes for anonymous shared mappings with fork"
    tester = "4_fork_tests/test_26.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test27(Xv6Test):
    name = "test_27"
    description = (
        "Validates child unmapping a private anon map does not affect the parent"
    )
    tester = "4_fork_tests/test_27.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test28(Xv6Test):
    name = "test_28"
    description = (
        "Validates child unmapping a shared anon map does not affect the parent"
    )
    tester = "4_fork_tests/test_28.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test29(Xv6Test):
    name = "test_29"
    description = "Remaps an anonymous mapping (grow, but does not need to move)"
    tester = "5_wremap_tests/test_29.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test30(Xv6Test):
    name = "test_30"
    description = "Remaps an anonymous mapping (shrink twice)"
    tester = "5_wremap_tests/test_30.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test31(Xv6Test):
    name = "test_31"
    description = "Remaps an anonymous mapping that needs to be moved"
    tester = "5_wremap_tests/test_31.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test32(Xv6Test):
    name = "test_32"
    description = "Validates memory access in a map that is shrinked"
    tester = "5_wremap_tests/test_32.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test33(Xv6Test):
    name = "test_33"
    description = "Validates memory access in a map that moved due to remap"
    tester = "5_wremap_tests/test_33.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test34(Xv6Test):
    name = "test_34"
    description = "Shrinks an anonymous map and then unmaps it"
    tester = "5_wremap_tests/test_34.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test35(Xv6Test):
    name = "test_35"
    description = "Remaps an anonymous mapping that requires moving and unmaps it"
    tester = "5_wremap_tests/test_35.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


class test36(Xv6Test):
    name = "test_36"
    description = "Remap an file-backed mapping"
    tester = "5_wremap_tests/test_36.c"
    make_qemu_args = "CPUS=1"
    point_value = 1
    failure_pattern = "Segmentation Fault"


from testing.runtests import main

main(
    Xv6Build,
    all_tests=[
        test1,
        test2,
        test3,
        test4,
        test5,
        test6,
        test7,
        test8,
        test9,
        test10,
        test11,
        test12,
        test13,
        test14,
        test15,
        test16,
        test17,
        test18,
        test19,
        test20,
        test21,
        test22,
        test23,
        test24,
        test25,
        test26,
        test27,
        test28,
        test29,
        test30,
        test31,
        test32,
        test33,
        test34,
        test35,
        test36,
    ],
    # Add your test groups here
    starter=[test1, test2, test3],
    wmap=[test4, test5, test6, test7, test8, test9, test10, test11, test12, test13],
    wunmap=[test14, test15, test16, test17, test18, test19, test20],
    fork=[test21, test22, test23, test24, test25, test26, test27, test28],
    wremap=[test29, test30, test31, test32, test33, test34, test35, test36],
    # End of test groups
)
