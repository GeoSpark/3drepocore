/**
 *  Copyright (C) 2014 3D Repo Ltd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "repo_graph_scene.h"

//------------------------------------------------------------------------------
//
// Constructors
//
//------------------------------------------------------------------------------

repo::core::RepoGraphScene::RepoGraphScene(
	const aiScene* scene,
	const std::map<std::string, RepoNodeAbstract*>& textures)
	: RepoGraphAbstract()
{
    //--------------------------------------------------------------------------
    // Textures
    std::map<std::string, RepoNodeAbstract*>::const_iterator tex_it;
    for (tex_it = textures.begin(); tex_it != textures.end(); ++tex_it)
    {
        RepoNodeAbstract *texture = tex_it->second;
        nodesByUniqueID.insert(std::make_pair(texture->getUniqueID(), texture));
    }

    //--------------------------------------------------------------------------
	// Materials
	//
	// Warning: Default material might not be attached to anything,
	// hence it would not be returned by a call to getNodes().
	if (scene->HasMaterials())
	{
		materials.reserve(scene->mNumMaterials);
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			aiString name;
			scene->mMaterials[i]->Get(AI_MATKEY_NAME, name);

			RepoNodeAbstract* material = new RepoNodeMaterial(
				scene->mMaterials[i],
				textures,
				name.data);
			materials.push_back(material);
			nodesByUniqueID.insert(std::make_pair(material->getUniqueID(), material));
		}
	}

    //--------------------------------------------------------------------------
	// Meshes
	if (scene->HasMeshes())
	{
		meshes.reserve(scene->mNumMeshes);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			RepoNodeAbstract* mesh = new RepoNodeMesh(
				REPO_NODE_API_LEVEL_1,
				scene->mMeshes[i],
				materials);
			meshes.push_back(mesh);
			nodesByUniqueID.insert(std::make_pair(mesh->getUniqueID(), mesh));
		}
	}

    //--------------------------------------------------------------------------
	// Cameras
	std::map<std::string, RepoNodeAbstract *> camerasMap;
	if (scene->HasCameras())
	{
		cameras.reserve(scene->mNumCameras);
		for (unsigned int i = 0; i < scene->mNumCameras; ++i)
		{
			std::string cameraName(scene->mCameras[i]->mName.data);
			RepoNodeAbstract *camera = new RepoNodeCamera(scene->mCameras[i]);
			cameras.push_back(camera);
			camerasMap.insert(std::make_pair(cameraName, camera));
			nodesByUniqueID.insert(std::make_pair(camera->getUniqueID(), camera));
		}
	}

    //--------------------------------------------------------------------------
	// TODO: Animations
	if (scene->HasAnimations())
	{

	}

    //--------------------------------------------------------------------------
	// TODO: Lights
	if (scene->HasLights())
	{

	}

	// TODO: Bones

    //--------------------------------------------------------------------------
	// Recursively converts aiNode and all of its children to a hierarchy
	// of RepoNodeTransformations. Call with root node of aiScene.
	// RootNode will be the first entry in transformations vector.
    rootNode = new RepoNodeTransformation(scene->mRootNode, meshes, camerasMap,
                                          transformations,
										  metadata);

    std::vector<RepoNodeAbstract *>::iterator it;
    for (it = transformations.begin(); it != transformations.end(); ++it)
    {
//	for each (RepoNodeAbstract* transformation in transformations)
        nodesByUniqueID.insert(std::make_pair((*it)->getUniqueID(),
                                              (*it)));
    }

    for (it = metadata.begin(); it != metadata.end(); ++it)
    {
        nodesByUniqueID.insert(std::make_pair((*it)->getUniqueID(),
                                              (*it)));
    }

}


repo::core::RepoGraphScene::RepoGraphScene(
	const std::vector<mongo::BSONObj>& collection) : RepoGraphAbstract()
{
	// To retrieve a graph, first identify a root node.
	// The very root normally does not have any parents, but this has to be
	// made more general as in the future it will be possible to retrieve a
	// subgraph from the scene in which case the root of the subgraph will
	// have his parents, however, will be the top-most node in the subgraph.
	//
	// This can be achieved through set intersections from paths.
	// The root node of the subgraph is the one which is in the retrieved
	// collection of objects and is referenced the most times by the nodes.
	// set_intersection on paths can deliver the most occurrences.

    std::map<boost::uuids::uuid, RepoNodeAbstract *> nodesBySharedID;
    for (std::vector<mongo::BSONObj>::const_iterator it = collection.begin();
         it != collection.end();
         ++it)
    {
        mongo::BSONObj obj = *it;
		RepoNodeAbstract *node = NULL;

		std::string nodeType = obj.getField(REPO_NODE_LABEL_TYPE).str();

		if (REPO_NODE_TYPE_TRANSFORMATION == nodeType)
		{
			node = new RepoNodeTransformation(obj);
			transformations.push_back(node);
		}
		else if (REPO_NODE_TYPE_MESH == nodeType)
		{
			node = new RepoNodeMesh(obj);
			meshes.push_back(node);
		}
		else if (REPO_NODE_TYPE_MATERIAL == nodeType)
		{
			node = new RepoNodeMaterial(obj);
			materials.push_back(node);
		}
		else if (REPO_NODE_TYPE_TEXTURE == nodeType)
		{
            RepoNodeTexture *tex = new RepoNodeTexture(obj);
			textures.push_back(tex);
			node = tex;
		}
		else if (REPO_NODE_TYPE_CAMERA == nodeType)
		{
			node = new RepoNodeCamera(obj);
			cameras.push_back(node);
		}
        else if (REPO_NODE_TYPE_REFERENCE == nodeType)
        {
            node = new RepoNodeReference(obj);
            references.push_back(node);
        }
        else if (REPO_NODE_TYPE_METADATA == nodeType)
        {
            node = new RepoNodeMetadata(obj);
            metadata.push_back(node);
        }

        //----------------------------------------------------------------------
		if (!obj.hasField(REPO_NODE_LABEL_PARENTS))
			rootNode = node;

        //----------------------------------------------------------------------
		// Skip objects of unrecognized type
		if (node)
		{
			nodesByUniqueID.insert(std::make_pair(node->getUniqueID(), node));
			// TODO: take care of multiple objects that have the same shared ID.
			nodesBySharedID.insert(std::make_pair(node->getSharedID(), node));
		}
		else
		{
			std::cerr << "Unrecognized node type" << std::endl;
			//RepoILogger::getInstance().log(repo::REPO_WARNING, "Node of unrecognized type.");
		}
	}

    //--------------------------------------------------------------------------
	// Build the parental graph.
	buildGraph(nodesBySharedID);
}


//------------------------------------------------------------------------------
//
// Destructor
//
//------------------------------------------------------------------------------

repo::core::RepoGraphScene::~RepoGraphScene()
{
    std::set<RepoNodeAbstract*> nodes = getNodes();
    std::set<RepoNodeAbstract*>::iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it)
    {
        delete *it;
	}
}

void repo::core::RepoGraphScene::append(RepoNodeAbstract *thisNode, RepoGraphAbstract *thatGraph)
{
    RepoGraphAbstract::append(thisNode, thatGraph);
    RepoGraphScene *thatScene = dynamic_cast<RepoGraphScene*>(thatGraph);
    if (thatScene)
    {
        materials.insert(materials.end(), thatScene->materials.begin(), thatScene->materials.end());
        meshes.insert(meshes.end(), thatScene->meshes.begin(), thatScene->meshes.end());
        transformations.insert(transformations.end(), thatScene->transformations.begin(), thatScene->transformations.end());
        textures.insert(textures.end(), thatScene->textures.begin(), thatScene->textures.end());
        cameras.insert(cameras.end(), thatScene->cameras.begin(), thatScene->cameras.end());
        references.insert(references.end(), thatScene->references.begin(), thatScene->references.end());
        metadata.insert(metadata.end(), thatScene->metadata.begin(), thatScene->metadata.end());
        thatScene->clear();
    }
    thatGraph->clear();
}


//------------------------------------------------------------------------------
//
// Export
//
//------------------------------------------------------------------------------

void repo::core::RepoGraphScene::toAssimp(aiScene *scene) const
{
    assert(NULL != scene);

    //--------------------------------------------------------------------------
	// Textures
	std::map<const RepoNodeAbstract *, std::string> texturesMapping;
    for (std::vector<RepoNodeTexture *>::const_iterator it = textures.begin();
         it != textures.end(); ++it)
    {
        RepoNodeAbstract *texture = *it;
		texturesMapping.insert(std::make_pair(texture, texture->getName()));
    }

    //--------------------------------------------------------------------------
	// Materials
	aiMaterial **mMaterials = new aiMaterial*[materials.size()];

	// Materials mapping map stores the index into the material array of
	// individual materials. This is in turn passed to meshes.
	std::map<const RepoNodeAbstract *, unsigned int> materialsMapping;
	if (NULL != mMaterials)
	{
        for (size_t i = 0; i < materials.size(); ++i)
		{
			aiMaterial *material = new aiMaterial();
			((RepoNodeMaterial *) materials[i])->toAssimp(texturesMapping,
				material);
			mMaterials[i] = material;
			materialsMapping.insert(std::make_pair(materials[i], i));
		}
		scene->mMaterials = mMaterials;
		scene->mNumMaterials = materials.size();
	}

    //--------------------------------------------------------------------------
	// Meshes
	aiMesh **mMeshes = new aiMesh*[meshes.size()];

	// Meshes mapping map stores the index into the meshes array of
	// individual meshes. This is in turn passed to transformations.
	std::map<const RepoNodeAbstract *, unsigned int> meshesMapping;
	if (NULL != mMeshes)
	{
		for (unsigned int i = 0; i < meshes.size(); ++i)
		{
			aiMesh *mesh = new aiMesh();
			((RepoNodeMesh *) meshes[i])->toAssimp(materialsMapping, mesh);
			mMeshes[i] = mesh;
			meshesMapping.insert(std::make_pair(meshes[i], i));
		}
		scene->mMeshes = mMeshes;
		scene->mNumMeshes = meshes.size();
	}



    //--------------------------------------------------------------------------
	// Cameras
	// Unlike meshes, cameras are not pointed to by index from transformations.
	// Instead, corresponding camera shares the same name with transformation in Assimp.
	aiCamera ** mCameras = new aiCamera*[cameras.size()];
	if (NULL != mCameras)
	{
		for (unsigned int i = 0; i < cameras.size(); ++i)
		{
			aiCamera *camera = new aiCamera();
			((RepoNodeCamera *) cameras[i])->toAssimp(camera);
			mCameras[i] = camera;
		}
		scene->mNumCameras = cameras.size();
		if (cameras.size())
			scene->mCameras = mCameras;
	}





    //--------------------------------------------------------------------------
	// Transformations
	std::map<const RepoNodeAbstract *, aiNode *> nodesMapping;
	for (unsigned int i = 0; i < transformations.size(); ++i)
	{
		aiNode *node = new aiNode();
		((RepoNodeTransformation *) transformations[i])->toAssimp(
			meshesMapping, node);
		nodesMapping.insert(std::make_pair(transformations[i], node));
	}


    //--------------------------------------------------------------------------
	// Recursively populate the tree hierarchy to aiNodes
	((RepoNodeTransformation*) rootNode)->toAssimp(nodesMapping, 0);

    //--------------------------------------------------------------------------
	// Assign root node if any
    std::map<const RepoNodeAbstract*, aiNode*>::iterator it =
            nodesMapping.find(rootNode);
	if (nodesMapping.end() != it)
		scene->mRootNode = it->second;
}

//------------------------------------------------------------------------------
//
// Getters
//
//------------------------------------------------------------------------------

std::vector<std::string> repo::core::RepoGraphScene::getNamesOfMeshes() const
{
	std::vector<std::string> names(meshes.size());
	for (unsigned int i = 0; i < names.size(); ++i)
		names[i] = meshes[i]->getName();
	return names;
}

//------------------------------------------------------------------------------
//
// Protected
//
//------------------------------------------------------------------------------

void repo::core::RepoGraphScene::clear()
{
    RepoGraphAbstract::clear();

    materials.clear();
    meshes.clear();
    textures.clear();
    transformations.clear();
    cameras.clear();
    references.clear();
    metadata.clear();
}
