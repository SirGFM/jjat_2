""" Reads 'misc/collision.json' and generates the code to handle collisions as described in it.
"""

import json
import sys

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
        out_file.write('/* Collision group \'{}\' */ \n'.format(k))

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

            # Select the handler ('CASE' handles being triggered by the first or second case differently)
            if function is not None:
                handler = 'CASE'
            else:
                handler = 'IGNORE'

            # After retrieving both lists, setup the collision code
            for a in typeAList:
                for b in typeBList:
                    if (b, a) in tuples:
                        # Ignore since case is already handled
                        # Do add it to the tuples, to catch any error
                        tuples.append((a, b))
                        continue
                    elif (a, b) in tuples:
                        print "Found repeated case ({}, {}) in {}".format(a, b, k)
                        return 6
                    elif a != b:
                        out_file.write('{}({}, {})\n'.format(handler, a, b))
                    else:
                        out_file.write('SELFCASE({})\n'.format(a))
                    tuples.append((a, b))
            if function is not None:
                out_file.write('    if (node1.pChild == node2.pChild) {\n')
                out_file.write('        /* Filter out self collision */\n')
                out_file.write('    }\n')
                out_file.write('    else if (isFirstCase) {\n')
                out_file.write('        erv = {}(&node1, &node2);\n'.format(function))
                out_file.write('    }\n')
                out_file.write('    else {\n')
                out_file.write('        erv = {}(&node2, &node1);\n'.format(function))
                out_file.write('    }\n')
            else:
                out_file.write('    erv = ERR_OK;\n')
            out_file.write('break;\n')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print 'Expected two arguments!'
        print 'Usage: {} json_filename output_filename'.format(sys.argv[0])
        sys.exit(1)

    fp = None
    if sys.argv[2] != 'stdout':
        try:
            fp = open(sys.argv[2], 'wt')
        except Exception as e:
            print 'Failed to open output file: {}'.format(e)
            sys.exit(2)
    else:
        fp = sys.stdout

    fp.write('/**\n' +
             ' * @file {}\n'.format(sys.argv[2]) +
             ' *\n' +
             ' * File generated from \'misc/collision.json\' to simplify handling collisions\n' +
             ' *\n' +
             ' * DO NOT EDIT MANUALLY\n' +
             ' */\n\n'
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
             '        erv = GFMRV_INTERNAL_ERROR;\n' +
             '#  endif\n' +
             '    }\n')
    fp.write('} /* switch (MERGE_TYPES(node1.type, node2.type)) */\n')
    fp.write('ASSERT(erv == ERR_OK, erv);\n')
    fp.close()

    sys.exit(rv)

