// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
//
// created by Dean Wadsworth aka Varmint Dec 31 2007

#ifndef __VOLUME_LIGHT_SCENE_NODE_H_INCLUDED__
#define __VOLUME_LIGHT_SCENE_NODE_H_INCLUDED__

#include "IVolumeLightSceneNode.h"
#include "SMeshBuffer.h"

namespace irr
{
namespace scene
{
	class CVolumeLightSceneNode : public IVolumeLightSceneNode
	{
	public:

		//! constructor
		CVolumeLightSceneNode(ISceneNode* parent, ISceneManager* mgr, s32 id,
			const u32 subdivU = 32, const u32 subdivV = 32,
			const video::SColor foot = video::SColor(51, 0, 230, 180),
			const video::SColor tail = video::SColor(0, 0, 0, 0),
			const core::vector3df& position = core::vector3df(0,0,0),
			const core::vector3df& rotation = core::vector3df(0,0,0),
			const core::vector3df& scale = core::vector3df(1.0f, 1.0f, 1.0f));

		virtual void OnRegisterSceneNode();

		//! renders the node.
		virtual void render();

		//! returns the axis aligned bounding box of this node
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		//! returns the material based on the zero based index i. To get the amount
		//! of materials used by this scene node, use getMaterialCount().
		//! This function is needed for inserting the node into the scene hirachy on a
		//! optimal position for minimizing renderstate changes, but can also be used
		//! to directly modify the material of a scene node.
		virtual video::SMaterial& getMaterial(u32 i);

		//! returns amount of materials used by this scene node.
		virtual u32 getMaterialCount() const;

		//! Returns type of the scene node
		virtual ESCENE_NODE_TYPE getType() const { return ESNT_CUBE; }

		//! Writes attributes of the scene node.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options=0) const;

		//! Reads attributes of the scene node.
		virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options=0);

		//! Creates a clone of this scene node and its children.
		virtual ISceneNode* clone(ISceneNode* newParent=0, ISceneManager* newManager=0);

		virtual void setSubDivideU (const u32 inU);
		virtual void setSubDivideV (const u32 inV);

		virtual u32 getSubDivideU () const { return SubdivideU; }
		virtual u32 getSubDivideV () const { return SubdivideV; }

		virtual void setFootColour(const video::SColor inColouf);
		virtual void setTailColour(const video::SColor inColouf);

		virtual video::SColor getFootColour () const { return FootColour; }
		virtual video::SColor getTailColour () const { return TailColour; }

	private:
		void addToBuffer(video::S3DVertex v);
		void constructLight();

		SMeshBuffer Buffer;

		f32  LPDistance;		// Distance to hypothetical lightsource point -- affects fov angle

		u32  SubdivideU;		// Number of subdivisions in U and V space.
		u32  SubdivideV;		// Controls the number of "slices" in the volume.
		// NOTE : Total number of polygons = 2 + ((SubdivideU + 1) + (SubdivideV + 1)) * 2
		// Each slice being a quad plus the rectangular plane at the bottom.

		video::SColor FootColour;		// Color at the source
		video::SColor TailColour;		// Color at the end.

		core::vector3df LightDimensions; // LightDimensions.Y Distance of shooting -- Length of beams
										 // LightDimensions.X and LightDimensions.Z determine the size/dimension of the plane
	};

} // end namespace scene
} // end namespace irr

#endif
