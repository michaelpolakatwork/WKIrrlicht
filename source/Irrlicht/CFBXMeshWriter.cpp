//This software contains Autodesk® FBX® code developed by Autodesk, Inc. Copyright 2008 Autodesk, Inc. 
//All rights, reserved. Such code is provided "as is" and Autodesk, Inc. disclaims any and all warranties, 
//whether express or implied, including without limitation the implied warranties of merchantability,
//fitness for a particular purpose or non-infringement of third party rights.
//In no event shall Autodesk, Inc. be liable for any direct, indirect, incidental, special, exemplary,
//or consequential damages (including, but not limited to, procurement of substitute goods or services;
//loss of use, data, or profits; or business interruption) however caused and on any theory of liability,
//whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of such code."

// Adapted for irrlicht by vonLeebpl vonleebpl(at)gmail.com

// Modified version with rigging/skinning support
// it's ugly way of calling collada writer first creating fbx scene 
// and import collada and then export with native fbx exporter

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_FBX_WRITER_

#include "CFBXMeshWriter.h"
#include "IFileSystem.h"
#include "ISceneManager.h"
#include "CColladaMeshWriter.h"
#include <iostream>
#include <fbxsdk.h>
#include "os.h"



namespace irr
{
namespace scene
{

	CFBXMeshWriter::CFBXMeshWriter(ISceneManager* smgr, video::IVideoDriver* driver, io::IFileSystem* fs)
		:_smgr(smgr), VideoDriver(driver), FileSystem(fs)
	{
		#ifdef _DEBUG
		setDebugName("CColladaMeshWriter");
		#endif

		if (VideoDriver)
			VideoDriver->grab();

		if (FileSystem)
			FileSystem->grab();

		
	}


	bool CFBXMeshWriter::writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags)
	{
		if (!file)
			return false;
		// create temp collada file
		std::remove("temp.dae");
		IColladaMeshWriter* ColladaMeshWr = (IColladaMeshWriter*)_smgr->createMeshWriter(EMWT_COLLADA);

		if (!ColladaMeshWr)
			return false;
		io::IWriteFile* _file = FileSystem->createAndWriteFile(L"temp.dae");

		int ret = ColladaMeshWr->writeMesh(_file, mesh, flags);

		_file->drop();
		ColladaMeshWr->drop();

		//now create fbx scene and read collada dae file

		// create a SdkManager
		FbxManager* lSdkManager = FbxManager::Create();

		// create an IOSettings object
		FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);

		// set some IOSettings options 
		ios->SetBoolProp(IMP_FBX_MATERIAL, true);
		ios->SetBoolProp(IMP_FBX_TEXTURE, true);
		ios->SetBoolProp(EXP_FBX_EMBEDDED, true);
		ios->SetBoolProp(EXP_FBX_SHAPE, true);
		ios->SetBoolProp(EXP_FBX_GOBO, true);
		ios->SetBoolProp(EXP_FBX_ANIMATION, true);
		ios->SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
		
		// create an empty scene
		FbxScene* lScene = FbxScene::Create(lSdkManager, "");

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Initialize the importer by providing a filename and the IOSettings to use
		lImporter->Initialize("temp.dae"); // , -1, ios);

		// Import the scene.
		ret = lImporter->Import(lScene);
		
		bool lStatus;
		if (ret) // success importing, verify scene integrity
		{
			FbxStatus status;
			FbxArray< FbxString*> details;
			FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(lScene), &status, &details);
			lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
			bool lNotify = (!lStatus && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
			if (lNotify)
			{
				os::Printer::log("********************************************************************************", ELL_DEBUG);
				if (details.GetCount())
				{
					os::Printer::log("Scene integrity verification failed with the following errors:", ELL_DEBUG);
					for (int i = 0; i < details.GetCount(); i++)
						os::Printer::log(details[i]->Buffer(), ELL_DEBUG);

					FbxArrayDelete<FbxString*>(details);
				}

				if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
				{
					os::Printer::log("   The importer was able to read the file but with errors.", ELL_WARNING);
					os::Printer::log("   Loaded scene may be incomplete.", ELL_WARNING);
					os::Printer::log("   Last error message: ", lImporter->GetStatus().GetErrorString(), ELL_WARNING);
				}
			}
		}
		else
		{
			os::Printer::log("DAE Import failed", ELL_ERROR);
			return false;
		}
		
		// Destroy the importer.
		lImporter->Destroy();

		// Create an exporter.
		FbxExporter* lExporter = FbxExporter::Create(lSdkManager, "");

		int pFileFormat = lSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();
		//Try to export in ASCII if possible
		int lFormatIndex, lFormatCount = lSdkManager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (lSdkManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = lSdkManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char* lASCII = "ascii";
				if (lDesc.Find(lASCII) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
		
		//damn conversion needed; fbx is not build with UNICODE support
		char * fileName = new char[250];
		const wchar_t*  fn = file->getFileName().c_str();
		wcstombs(fileName, fn, wcslen(fn));
		fileName[wcslen(fn)] = '\0';

		// need to drop and delete as irrlicht keeps file for open
		file->drop();
		std::remove(fileName);

		// Initialize the exporter by providing a filename.
		if (lExporter->Initialize(fileName, pFileFormat, lSdkManager->GetIOSettings()) == false)
		{
			os::Printer::log("Call to FbxExporter::Initialize() failed.", ELL_ERROR);
			os::Printer::log("Error returned: ", lExporter->GetStatus().GetErrorString(), ELL_ERROR);
			return false;
		}

		// Export the scene.
		lStatus = lExporter->Export(lScene);
		if (!lStatus)
			os::Printer::log("Call to FbxExporter::Export() failed.", ELL_ERROR);

		//cleanup
		lExporter->Destroy();
		std::remove("temp.dae");

		return lStatus;
	}

	// return type of mesh writer
	EMESH_WRITER_TYPE CFBXMeshWriter::getType() const
	{
		return EMWT_FBX;
	}

	CFBXMeshWriter::~CFBXMeshWriter()
	{
		if (VideoDriver)
			VideoDriver->drop();

		if (FileSystem)
			FileSystem->drop();
	}

} //namespace scene
} //namespace irr
#endif

