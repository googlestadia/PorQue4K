#!/usr/bin/python3
import xml.etree.ElementTree as ET
import os.path
import sys

# All the Visual Studio generated files use this namespace...for now!
ET.register_namespace('', "http://schemas.microsoft.com/developer/msbuild/2003")
ns = {'defns': 'http://schemas.microsoft.com/developer/msbuild/2003'}

if len(sys.argv) < 2:
    print("Path to cmake build folder required, exiting...")
    exit()

generatedBuildPath = sys.argv[1]

solutionFilePath = os.path.join(generatedBuildPath, 'VulkanPorQue4K.sln')
with open(solutionFilePath, "r") as solutionFile:
    data = solutionFile.read().replace("x64", "GGP")

with open(solutionFilePath, "w") as solutionFile:
    solutionFile.write(data)

# This really needs to be processed so the solution loads...
allBuildProjectFilePath = os.path.join(generatedBuildPath, 'ALL_BUILD.vcxproj')
with open(allBuildProjectFilePath, "r") as allBuildProjectFile:
    data = allBuildProjectFile.read().replace("x64", "GGP")

with open(allBuildProjectFilePath, "w") as allBuildProjectFile:
    allBuildProjectFile.write(data)

tree = ET.parse(allBuildProjectFilePath)
root = tree.getroot()

for propGroup in root.findall('defns:PropertyGroup', ns):
    if propGroup.get('Label') == 'Globals':
        propGroup.remove(propGroup.find('defns:Keyword', ns))
    elif propGroup.get('Label') == 'Configuration':
        propGroup.remove(propGroup.find('defns:PlatformToolset', ns)) # optional?

tree.write(allBuildProjectFilePath)


vkexProjectFilePath = os.path.join(generatedBuildPath, 'src/vkex/vkex.vcxproj')
with open(vkexProjectFilePath, "r") as vkexProjectFile:
    data = vkexProjectFile.read().replace("x64", "GGP")

with open(vkexProjectFilePath, "w") as vkexProjectFile:
    vkexProjectFile.write(data)

tree = ET.parse(vkexProjectFilePath)
root = tree.getroot()

for propGroup in root.findall('defns:PropertyGroup', ns):
    if propGroup.get('Label') == 'Globals':
        propGroup.remove(propGroup.find('defns:Keyword', ns))
    elif propGroup.get('Label') == 'Configuration':
        propGroup.remove(propGroup.find('defns:PlatformToolset', ns)) # optional?
        if propGroup.get('Condition').find('Debug') != -1:
            useDebugLib = ET.Element('UseDebugLibraries')
            useDebugLib.text = 'true'
            useDebugLib.tail = propGroup.tail + '  '
            propGroup.append(useDebugLib)
        elif propGroup.get('Condition').find('Release') != -1:
            wholeProgOpt = ET.Element('WholeProgramOptimization')
            wholeProgOpt.text = 'true'
            wholeProgOpt.tail = propGroup.tail + '  '
            propGroup.append(wholeProgOpt)

for itemDefGroup in root.findall('defns:ItemDefinitionGroup', ns):
    for compileNode in itemDefGroup.findall('defns:ClCompile', ns):
        for objFileName in compileNode.findall('defns:ObjectFileName', ns):
            objFileName.text += '%(Filename)'
        
        useMultiTool = ET.Element('UseMultiToolTask')
        useMultiTool.text = 'true'
        useMultiTool.tail = compileNode.tail + '  '
        compileNode.append(useMultiTool)

        debugConfig = (itemDefGroup.get('Condition').find('Debug') != -1)

        optimization = ET.Element('Optimization')
        if debugConfig:
            optimization.text = 'Disabled'
        else:
            optimization.text = 'MaxSpeed'
        optimization.tail = compileNode.tail + '  '
        compileNode.append(optimization)

tree.write(vkexProjectFilePath)


appProjectFilePath = os.path.join(generatedBuildPath, 'src/app/4KApp.vcxproj')
with open(appProjectFilePath, "r") as appProjectFile:
    data = appProjectFile.read().replace("x64", "GGP")

with open(appProjectFilePath, "w") as appProjectFile:
    appProjectFile.write(data)

tree = ET.parse(appProjectFilePath)
root = tree.getroot()

for propGroup in root.findall('defns:PropertyGroup', ns):
    if propGroup.get('Label') == 'Globals':
        propGroup.remove(propGroup.find('defns:Keyword', ns))
    elif propGroup.get('Label') == 'Configuration':
        propGroup.remove(propGroup.find('defns:PlatformToolset', ns)) # optional?
        if propGroup.get('Condition').find('Debug') != -1:
            useDebugLib = ET.Element('UseDebugLibraries')
            useDebugLib.text = 'true'
            useDebugLib.tail = propGroup.tail + '  '
            propGroup.append(useDebugLib)
        elif propGroup.get('Condition').find('Release') != -1:
            wholeProgOpt = ET.Element('WholeProgramOptimization')
            wholeProgOpt.text = 'true'
            wholeProgOpt.tail = propGroup.tail + '  '
            propGroup.append(wholeProgOpt)

for itemDefGroup in root.findall('defns:ItemDefinitionGroup', ns):
    debugConfig = (itemDefGroup.get('Condition').find('Debug') != -1)

    for compileNode in itemDefGroup.findall('defns:ClCompile', ns):
        for objFileName in compileNode.findall('defns:ObjectFileName', ns):
            objFileName.text += '%(Filename)'
        
        useMultiTool = ET.Element('UseMultiToolTask')
        useMultiTool.text = 'true'
        useMultiTool.tail = compileNode.tail + '  '
        compileNode.append(useMultiTool)

        optimization = ET.Element('Optimization')
        if debugConfig:
            optimization.text = 'Disabled'
        else:
            optimization.text = 'MaxSpeed'
        optimization.tail = compileNode.tail + '  '
        compileNode.append(optimization)

        if debugConfig == False:
            funcLevelLink = ET.Element('FunctionLevelLinking')
            funcLevelLink.text = 'true'
            funcLevelLink.tail = compileNode.tail + '  '
            compileNode.append(funcLevelLink)

            instFuncs = ET.Element('IntrinsicFunctions')
            instFuncs.text = 'true'
            instFuncs.tail = compileNode.tail + '  '
            compileNode.append(instFuncs)

    for linkNode in itemDefGroup.findall('defns:Link', ns):
        linkNode.remove(linkNode.find('defns:GenerateDebugInformation', ns))
        linkNode.remove(linkNode.find('defns:ProgramDataBaseFile', ns))
        linkNode.remove(linkNode.find('defns:SubSystem', ns))

        for addlDepsNode in linkNode.findall('defns:AdditionalDependencies', ns):
            depsList = addlDepsNode.text.split(';')
            newDepsList = ""
            for dep in depsList:
                if dep.find('libvkex') != -1:
                    vkexDir, vkexFile = os.path.split(dep)
                    newDepsList += vkexFile + ";"
                elif dep.find('libglfw') != -1:
                    glfwDir, glfwFile = os.path.split(dep)
                    newDepsList += glfwFile + ';'
                elif dep.find('-l') != -1:
                    newLibName = 'lib' + dep[2:] + '.so;'
                    newDepsList += newLibName
                else:
                    newDepsList += dep + ';'
            addlDepsNode.text = newDepsList

        for addlLibDirsNode in linkNode.findall('defns:AdditionalLibraryDirectories', ns):
            addlLibDirsNode.text = vkexDir + ';' + glfwDir + ';' + addlLibDirsNode.text

        if debugConfig == False:
            optRefs = ET.Element('OptimizeReferences')
            optRefs.text = 'true'
            optRefs.tail = linkNode.tail + '  '
            linkNode.append(optRefs)

tree.write(appProjectFilePath)
