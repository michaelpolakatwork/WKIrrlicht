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

#ifndef __IRR_FBX_MESH_WRITER_H_INCLUDED__
#define __IRR_FBX_MESH_WRITER_H_INCLUDED__

#include "IMeshWriter.h"
#include "IWriteFile.h"
#include "IVideoDriver.h"


#include "ISceneNode.h"
//#include "S3DVertex.h"

namespace irr
{
namespace io
{
	class IFileSystem;
}

namespace scene
{
	class CFBXMeshWriter : public IMeshWriter
	{
	public:

		CFBXMeshWriter(ISceneManager* smgr, video::IVideoDriver* driver, io::IFileSystem* fs);
		virtual ~CFBXMeshWriter();

		//! Returns the type of the mesh writer
		virtual EMESH_WRITER_TYPE getType() const _IRR_OVERRIDE_;

		//! writes a scene starting with the given node
		//virtual bool writeScene(io::IWriteFile* file, scene::ISceneNode* root, int writeRoot);

		//! writes a mesh
		virtual bool writeMesh(io::IWriteFile* file, scene::IMesh* mesh, s32 flags = EMWF_NONE) _IRR_OVERRIDE_;

	protected:

		ISceneManager* _smgr;
		io::IFileSystem* FileSystem;
		video::IVideoDriver* VideoDriver;
		io::path Directory;

	};
}
}
#endif