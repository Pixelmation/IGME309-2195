#include "Simplex\Physics\Octant.h"
using namespace Simplex;

Octant::Octant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax;

	uint iObjects = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < iObjects; i++)
	{
		Entity* pEntity = m_pEntityMngr->GetEntity(i);
		RigidBody* pRB = pEntity->GetRigidBody();

		lMinMax.push_back(pRB->GetMinGlobal());
		lMinMax.push_back(pRB->GetMaxGlobal());
	}

	RigidBody* pRigidBody = new RigidBody(lMinMax);

	vector3 halfWidth = pRigidBody->GetHalfWidth();

	float fMax = halfWidth.x;
	for (size_t i = 0; i < 3; i++)
	{
		if (fMax < halfWidth[i])
		{
			fMax = halfWidth[i];
		}
	}

	vector3 center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = fMax * 2.0f;
	m_v3Center = center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

Octant::Octant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

Octant::Octant(Octant const& other)
{
	m_nData = other.m_nData;
	m_lData = other.m_lData;
}

Octant& Octant::operator=(Octant const& other)
{
	if (this != &other)
	{
		Release();
		Init();
		Octant temp(other);
		Swap(temp);
	}
	return *this;
}

Octant::~Octant() 
{ 
	Release(); 
}

void Octant::Swap(Octant& other)
{
	std::swap(m_nData, other.m_nData);
	std::swap(m_lData, other.m_lData);
}

bool Octant::IsColliding(uint a_uRBIndex)
{
	return false;
}

void Octant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		for (size_t i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(a_v3Color);
		}

		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	else
	{
		for (size_t i = 0; i < m_uChildren; i++)
		{
			m_pChild[i]->Display(i, a_v3Color);
		}
	}


}

void Octant::Display(vector3 a_v3Color)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Octant::DisplayLeafs(vector3 a_v3Color)
{
}

void Octant::ClearEntityList(void)
{
	m_EntityList.clear();
}

void Octant::Subdivide(void)
{

}

Octant* Octant::GetChild(uint a_nChild)
{
	return nullptr;
}

Octant* Octant::GetParent(void)
{
	return nullptr;
}

bool Octant::IsLeaf(void)
{
	return false;
}

bool Octant::ContainsMoreThan(uint a_nEntities)
{
	int count = 0;
	int objectCount = m_pEntityMngr->GetEntityCount();
	for (size_t i = 0; i < objectCount; i++)
	{
		if (IsColliding(i))
		{
			count++;
		}
		if (count > a_nEntities)
		{
			return true;
		}
	}
	return false;
}

void Octant::KillBranches(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void Octant::ConstructTree(uint a_nMaxLevel)
{
	if (m_uLevel != 0) return;

	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	AssignIDtoEntity();

	ConstructList();
}

void Octant::AssignIDtoEntity(void)
{

}



void Octant::Release(void)
{
	if (m_uLevel == 0)
		KillBranches();

	m_uChildren = 0;
	m_fSize = 0.0f;
	m_lChild.clear();
}

void Octant::Init(void)
{
	m_nData = 0;
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = EntityManager::GetInstance();
	 
	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}

void Octant::ConstructList(void)
{
	for (size_t i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}