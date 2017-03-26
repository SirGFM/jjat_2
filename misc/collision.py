""" Reads 'misc/collision.json' and generates the code to handle collisions as described in it.
"""

import json
import os
import shutil
import sys
import tempfile

def output_collision(case, list_a, list_b, handler, function, out_file, tuples, reverse=False):
    # Remove ignored cases from the release version, since it's useless
    if function is None:
        out_file.write('#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))\n')

    for a in list_a:
        for b in list_b:
            if (a, b) in tuples:
                print "Found repeated case ({}, {}) in {}".format(a, b, case)
                return 6
            elif a != b:
                out_file.write('    {}({}, {})\n'.format(handler, a, b))
            else:
                out_file.write('    SELFCASE({})\n'.format(a))
            tuples.append((a, b))
    if function is not None:
        out_file.write('        if (node1.pChild != node2.pChild) {\n')
        out_file.write('            /* Filter out self collision */\n')
        if not reverse:
            out_file.write('            erv = {}(&node1, &node2);\n'.format(function))
        else:
            out_file.write('            erv = {}(&node2, &node1);\n'.format(function))
        out_file.write('        }\n')
    else:
        out_file.write('        erv = ERR_OK;\n')
    out_file.write('    break;\n')

    # Remove ignored cases from the release version, since it's useless
    if function is None:
        out_file.write('#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */\n')
    return 0

def main(json_filename, out_file):
    fp = None
    decoded = None

    # Decode the file
    try:
        fp = open(json_filename, 'rt')
        decoded = json.load(fp)
    except Exception as e:
        print "Failed to decode json file: {}".format(e)
        return 3
    finally:
        if fp is not None:
            fp.close()

    # Iterate through every collision group, generating its code
    tuples = []
    for k, v in decoded.iteritems():
        out_file.write('    /* Collision group \'{}\' */ \n'.format(k))

        try:
            typeAList = v['type_a']
            caseList = v['cases']
        except Exception as e:
            print "Error generating code for {}: {}".format(k, e)
            return 4

        for c in caseList:
            try:
                typeBList = c['type_b']
                function = c['function']
            except Exception as e:
                print "Error generating code for case {} in {}: {}".format(c, k, e)
                return 5
            # If 'reverse' isn't present, default to False!
            try:
                reverse = c['reverse']
            except:
                reverse = False
            # If 'auto_swap' isn't present, default to True! (mimics older behaviour)
            try:
                auto_swap = c['auto_swap']
            except:
                auto_swap = True

            # Select the handler ('CASE' handles being triggered by the first or second case differently)
            if function is not None:
                handler = 'CASE'
            else:
                handler = 'IGNORE'

            # After retrieving both lists, setup the collision code
            rv = output_collision(k, typeAList, typeBList, handler, function, out_file, tuples, reverse)
            if rv != 0:
                return rv
            if auto_swap:
                rv = output_collision(k, typeBList, typeAList, handler, function, out_file, tuples, not reverse)
                if rv != 0:
                    print 'Swapped case failed!'
                    return rv
    return 0

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print 'Expected two arguments!'
        print 'Usage: {} json_filename output_filename'.format(sys.argv[0])
        sys.exit(1)

    if sys.argv[2] != 'stdout':
        try:
            fp = tempfile.NamedTemporaryFile(mode='wt', delete=False)
            filepath = fp.name
        except Exception as e:
            print 'Failed to open output file: {}'.format(e)
            sys.exit(2)
    else:
        fp = sys.stdout

    fp.write('/**\n'
             ' * @file {}\n'.format(sys.argv[2]) +
             ' *\n'
             ' * File generated from \'misc/collision.json\' to simplify handling collisions\n'
             ' *\n'
             ' * DO NOT EDIT MANUALLY\n'
             ' */\n\n'
             '/* Avoid error if it\'s a self collision */'
             'erv = ERR_OK;\n'
             '/* Merge both types into a single one, so it\'s easier to compare */\n'
             'switch (MERGE_TYPES(node1.type, node2.type)) {\n')

    rv = main(sys.argv[1], fp)

    # Setup a default case on linux & debug, so unhandled collisions may be easily detected
    fp.write('    /* On Linux, a SIGINT is raised any time a unhandled collision\n' +
             '     * happens. When debugging, GDB will stop here and allow the user to\n' +
             '     * check which types weren\'t handled */\n' +
             '    default: {\n' +
             '#  if defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__))\n' +
             '        /* Unfiltered collision, do something about it */\n' +
             '        raise(SIGINT);\n' +
             '        erv = ERR_UNHANDLED_COLLISION;\n' +
             '#  endif /* defined(DEBUG) && !(defined(__WIN32) || defined(__WIN32__)) */\n' +
             '    }\n')
    fp.write('} /* switch (MERGE_TYPES(node1.type, node2.type)) */\n')
    fp.write('ASSERT(erv == ERR_OK, erv);\n')
    fp.close()

    # Move the file to its final destination
    if rv == 0 and sys.argv[2] != 'stdout':
        try:
            os.remove(sys.argv[2])
        except:
            pass
        try:
            shutil.move(filepath, sys.argv[2])
        except Exception as e:
            print 'Failed to create the output file: {}'.format(e)
            rv = 7
    elif sys.argv[2] != 'stdout':
        os.remove(filepath)

    sys.exit(rv)

