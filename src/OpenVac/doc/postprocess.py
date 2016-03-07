
import os
import re
from shutil import copyfile

INPUT_DIR = 'dox'
OUTPUT_DIR = 'html'
LAYOUT_DIR = 'layout'

################################# FUNCTIONS ######################################

def read(filename):
    f = open(INPUT_DIR + '/' + filename,'r')
    filedata = f.read()
    f.close()
    return filedata

def write(filename, filedata):
    f = open(OUTPUT_DIR + '/' + filename,'w')
    f.write(filedata)
    f.close()

def readLayout():
    f = open(LAYOUT_DIR + '/layout.htm','r')
    filedata = f.read()
    f.close()
    return filedata

def copyStyleFiles():
    for filename in ['OpenSans-Light.ttf',
                     'style.css',
                     'logo.png',
                     'teaser-vgc.png',
                     'teaser-vac.png']:
        copyfile(LAYOUT_DIR + '/' + filename, 
                 OUTPUT_DIR + '/' + filename)

def getPageTitle(filedata):
    match = re.search("<div class=\"title\">(.*?)</div>", filedata)
    if match:
        return match.groups()[0]

def getPageContent(filedata):
    match = re.search("<div class=\"contents\">(.*?)</div><!-- contents -->", filedata, re.DOTALL)
    if match:
        return match.groups()[0]

def makeIndexPage(filename):
    dox = read(filename)
    content = getPageContent(dox)
    out = readLayout()
    out = out.replace('<h1>$title</h1>', '')
    out = out.replace('$content', content)
    write(filename, out)

def makePage(filename):
    dox = read(filename)
    title = getPageTitle(dox)
    content = getPageContent(dox)
    out = readLayout()
    out = out.replace('$title', title)
    out = out.replace('$content', content)
    write(filename, out)

def processClass(filename):
    pass
    #replaceMainPageToHome(filename):
    #print filename#[5:]

def process(filename):
    if filename.endswith('.html'):
        replaceMainPageToHome(filename)

    if ( (filename.startswith('class') or filename.startswith('struct')) and
         filename.endswith('.html')  and
         not filename.endswith('-members.html')  and
         filename != 'classes.html'       
        ):
        processClass(filename)


############################### ACTUAL SCRIPT #################################

# Create output dir
if not os.path.exists(OUTPUT_DIR):
    os.makedirs(OUTPUT_DIR)

# Iterate over all files
#filenames = os.listdir(INPUT_DIR)
#filenames.sort()
#for filename in filenames:
#    if filename.endswith('.htm'):
#        # Print Doxygen title of all htm files
#        filedata = read(filename)
#        title = getPageTitle(filedata)
#        if title:
#            print title

copyStyleFiles()
makeIndexPage('index.htm')
makePage('gettingstarted.htm')
makePage('topology.htm')
makePage('geometry.htm')
makePage('algorithms.htm')
makePage('operators.htm')
#content = getPageContent(read('somepage.htm'))
#layout = readLayout()
#data = layout.replace('$content', content)
#write('index.htm', data)

#content = 
#        title = getPageTitle(filedata)
#        if title:
#            print title


#    process(filename)
