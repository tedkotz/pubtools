#!/usr/bin/env python3

#  .
#  ├── DIR1
#  │   └── file1.html
#  ├── DIR2
#  │   ├── file1.html
#  │   ├── file2.html
#  │   └── file3.html
#  └── file1.html
#
#  DIR1/file1.html
#  DIR2/file1.html
#  DIR2/file2.html
#  DIR2/file3.html
#  file1.html
#
# <h1>Table of Contents</h1>
# <ol>
#   <li>DIR1
#     <ol>
#       <li><a href="file1.html">TITLE1</a></li>
#   </ol></li>
#   <li>DIR2
#     <ol>
#       <li><a href="file1.html">TITLE1</a></li>
#       <li><a href="file2.html">TITLE2</a></li>
#       <li><a href="file3.html">TITLE3</a></li>
#   </ol></li>
#   <li><a href="file1.html">TITLE1</a></li>
# </ol>

import argparse
import sys
import re

def prettyPrintFilenameToString( filename ):
    # Remove extensions
    #  "70_2+2is4_BasicMath.dir" -> "70_2+2is4_BasicMath"
    retVal = str(filename).rsplit( '.', maxsplit=1)[0]

    # Remove number prefixed to force ordering
    #  "70_2+2is4_BasicMath" -> "2+2is4_BasicMath"
    retVal = re.sub( r'\A[0-9]*[-_/\ ]*', r'', retVal )

    # Split camel case, strings of digits or individual symbols
    # "2+2is4_BasicMath" -> "2 + 2 is 4 _ Basic Math"
    retVal = re.sub( r'([a-z])([^a-z ])', r'\1 \2', retVal )
    retVal = re.sub( r'([^ ])([^a-zA-Z0-9 ])', r'\1 \2', retVal )
    retVal = re.sub( r'([^a-zA-Z0-9 ])([^ ])', r'\1 \2', retVal )
    retVal = re.sub( r'([a-zA-Z])([0-9])', r'\1 \2', retVal )
    retVal = re.sub( r'([0-9])([a-zA-Z])', r'\1 \2', retVal )

    # Remove double spaces
    # Replace '-','_','\','/' with space
    # "2 + 2 is 4 _ Basic Math" -> "2 + 2 is 4 Basic Math"
    retVal = re.sub( r'[-_/\ ]+', ' ', retVal )

    # Capitalize first letter
    # retVal = retVal.capitalize()

    # Capitalize ever letter after as space
    #  "2 + 2 is 4 Basic Math" -> "2 + 2 Is 4 Basic Math"
    retVal = ' '.join(word.capitalize() for word in retVal.split(" "))

    # Return result
    return retVal

def extractTitleFromHtml(filename):
    try:
        with open(filename, 'r') as fin:
            retVal = None
            for line in fin:
                if( retVal is None ):
                    start = line.lower().find("<title>");
                    if -1 != start:
                        line = line[start+7:]
                        retVal= ""

                if(retVal is not None):
                    start = line.lower().find("</title>");
                    if -1 == start:
                        retVal = retVal + line.strip()
                    else:
                        return (retVal + line[:start].strip())
    except FileNotFoundError:
        pass

    return None

def getFileTitle(path, filename):
    title = extractTitleFromHtml(path + filename)
    if title is None:
        return prettyPrintFilenameToString(filename)
    else:
        return title

# python needs a more functional string token option
def splitStringAtFirstToken( line, delim ):
    substrings = line.split(delim, maxsplit=1)
    if 0 == len(substrings):
        return None, None
    elif 1 == len(substrings):
        return substrings[0], None
    else:
        return substrings[0], substrings[1]

def createSubdirListItem( out, inputIterator, basepath, indent, relpath, subdirname ):
    out.write(indent+"<li>"+prettyPrintFilenameToString(subdirname)+"\n")
    createOrderedList( out, inputIterator, basepath, indent+"  ", relpath + subdirname +'/' )
    out.write(indent+"</li>\n")

def createOrderedList( out, inputIterator, basepath, indent="", relpath="" ):
    if(inputIterator is not None):
        out.write(indent+"<ol>\n")
        subdir = None
        subdirname = ''
        for line in inputIterator:
            first, rest = splitStringAtFirstToken(line.strip(), '/')
            if( subdir is not None ):
                if (rest is not None) and (first == subdirname):
                    subdir.append(rest)
                else:
                    createSubdirListItem( out, subdir, basepath, indent+"  ", relpath, subdirname )
                    subdir=None

            # After processing old subdir check if we still have a line to process
            if( subdir is None ):
                if( rest is None ):
                    out.write(indent+'  <li><a href="'+relpath+first+'">'+getFileTitle(basepath+relpath, first)+'</a></li>\n')
                else:
                    # start new subdir
                    subdirname=first
                    subdir = [ rest ]

        # End for loop
        if( subdir is not None ):
            createSubdirListItem( out, subdir, basepath, indent+"  ", relpath, subdirname )
            subdir=None

        out.write(indent+"</ol>\n")

def createToc( out, inputIterator, basepath ):
    out.write("<h1>Table of Contents</h1>\n")
    createOrderedList(out, inputIterator, basepath)


def main( args ):
    if (1 == len(args.files)) and ('-' == args.files[0]):
        inputIterator = sys.stdin
    else:
        inputIterator = args.files

    if '-' == args.output:
        createToc(sys.stdout, inputIterator, args.basepath)
    else:
        with open(args.output, 'w') as out:
            createToc(out, inputIterator, args.basepath)



# program entry point ...
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('files', metavar='PATH', nargs='+',
                        help='PATH to add to list of links in table of contents. If - is only path it reads the list from stdin.')
    parser.add_argument('-o', '--output', metavar='FILE', default='-',
                        help='Write table of contents to FILE. By default or if set to - it writes to stdout.')
    parser.add_argument('-b', '--basepath', metavar='PATH', default='',
                        help='Base PATH to search for files when extracting titles.')
    main(parser.parse_args())

