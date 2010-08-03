#include <Opengl2.h>
#include <wx/wx.h>
#include "EditorCanvas.h"
#include <OrangeGrass.h>
#include <IOGGraphicsHelpers.h>
#include <IOGMath.h>


#define TIMER_ID    1000

BEGIN_EVENT_TABLE(CEditorCanvas, wxGLCanvas)
    EVT_TIMER(TIMER_ID, CEditorCanvas::OnTimer)
    EVT_SIZE(CEditorCanvas::OnSize)
    EVT_PAINT(CEditorCanvas::OnPaint)
    EVT_ERASE_BACKGROUND(CEditorCanvas::OnEraseBackground)
    EVT_KEY_DOWN( CEditorCanvas::OnKeyDown )
    EVT_KEY_UP( CEditorCanvas::OnKeyUp )
    EVT_ENTER_WINDOW( CEditorCanvas::OnEnterWindow )
	EVT_TOOLCMD( wxID_ANY, CEditorCanvas::OnToolCmdEvent )
	EVT_LEVELLOAD( wxID_ANY, CEditorCanvas::OnLevelLoadEvent )
	EVT_RESSWITCH( wxID_ANY, CEditorCanvas::OnResourceSwitch )
	EVT_ENTER_WINDOW( CEditorCanvas::OnMouseEnter )
	EVT_LEAVE_WINDOW( CEditorCanvas::OnMouseLeave )
	EVT_MOTION( CEditorCanvas::OnMouseMove )
	EVT_LEFT_DOWN( CEditorCanvas::OnLMBDown )
	EVT_LEFT_UP( CEditorCanvas::OnLMBUp )
	EVT_RIGHT_DOWN( CEditorCanvas::OnRMBDown )
	EVT_RIGHT_UP( CEditorCanvas::OnRMBUp )
	EVT_MOUSEWHEEL( CEditorCanvas::OnMouseWheel )
END_EVENT_TABLE()


MATRIX		m_mProjection;
MATRIX		m_mView;
IOGLevel*	m_pCurLevel = NULL;
IOGSgNode*  m_pCurNode = NULL;
IOGActor*   m_pPickedActor = NULL;
bool		m_bIntersectionFound = false;
Vec3		m_vIntersection;
Vec3		m_vCurRotation;
Vec3		m_vCurScaling(1);
std::string m_CurModelAlias;


/// @brief Constructor.
/// @param parent - parent window.
/// @param id - window ID.
/// @param pos - window position.
/// @param size - window size.
/// @param style - window style.
/// @param name - window name.
CEditorCanvas::CEditorCanvas (  wxWindow *parent, 
                                wxWindowID id,
                                const wxPoint& pos, 
                                const wxSize& size, 
                                long style, 
                                const wxString& name) : wxGLCanvas(parent, (wxGLCanvas*)NULL, id, pos, size, style|wxFULL_REPAINT_ON_RESIZE, name),
                                                        m_timer(this, TIMER_ID)
{
    m_init = false;
	m_bLoaded = false;
	GetEventHandlersTable()->AddEventHandler(EVENTID_RESSWITCH, this);
	GetEventHandlersTable()->AddEventHandler(EVENTID_TOOLCMD, this);
	GetEventHandlersTable()->AddEventHandler(EVENTID_LEVELLOAD, this);
    m_timer.Start(100);
	m_fCameraDistance = 400.0f;

    bRmb = bLmb = false;
    mouse_x = mouse_y = 0;

    m_fFineAngleStep = TO_RADIAN(2.0f);
    m_fCoarseAngleStep = TO_RADIAN(45.0f);

	m_bShowAABB = false;
	m_bMouseInWindow = true;
	m_bMouseMoved = false;
	m_bRedrawPatch = false;

	m_bIntersectionFound = false;
}


/// @brief Destructor.
CEditorCanvas::~CEditorCanvas()
{
}


/// @brief Render.
void CEditorCanvas::Render()
{
    wxPaintDC dc(this);
    dc.GetWindow()->GetHandle();

    if (!GetContext()) 
        return;

    SetCurrent();

    // Init OpenGL once, but after SetCurrent
    if (!m_init)
    {
        InitGL();
		GetResourceMgr()->Init("resources.xml");
		GetLevelManager()->Init("levels.yaml");
		m_init = true;
    }

	if (!m_bLoaded)
	{
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glFlush();
		SwapBuffers();
		LoadNextResource();
	}

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(m_mProjection.f);
	
	glMatrixMode(GL_MODELVIEW);
	m_mView = GetCamera()->Update();
	glLoadMatrixf(m_mView.f);

    GetActorManager()->Update(10);

	if (m_pCurLevel)
        m_pCurLevel->GetTerrain()->Render(m_mView);

    GetSceneGraph()->Render(m_mView);

    switch (GetToolSettings()->GetEditMode())
    {
    case EDITMODE_OBJECTS:
        {
            if (m_pCurNode)
            {
                m_pCurNode->Update(10);
                MATRIX mModelView = m_pCurNode->GetWorldTransform();
                MatrixMultiply(mModelView, mModelView, m_mView);
                m_pCurNode->GetRenderable()->Render(mModelView);
            }
        }
        break;
    }

    RenderHelpers();

    glFlush();
    SwapBuffers();
}


/// @brief Timer handler.
/// @param event - event structute.
void CEditorCanvas::OnTimer(wxTimerEvent& event)
{
	if (m_bMouseMoved)
	{
		Vec3 vPick = GetPickRay (mouse_x, mouse_y);
		Vec3 vPos = GetCamera()->GetPosition();
		Vec3 vVec = vPick - vPos;
		vVec.normalize();

		ToolSettings* pTool = GetToolSettings();
		switch (pTool->GetEditMode())
		{
		case EDITMODE_OBJECTS:
			{
                if (m_pCurLevel)
				{
					m_bIntersectionFound = m_pCurLevel->GetTerrain()->GetRayIntersection(vPos, vVec, &m_vIntersection);
					if (m_bIntersectionFound)
					{
						if (m_pCurNode)
						{
							m_pCurNode->SetPosition(m_vIntersection);
						}
						m_bRedrawPatch = true;
					}
				}
			}
			break;

		case EDITMODE_ADJUST:
			{
			}
			break;

		case EDITMODE_SETTINGS:
			{
			}
			break;
		}

		GetActorManager()->Update(10);

		m_bMouseMoved = false;
		Refresh();
	}
}


/// @brief Enter window handler.
/// @param event - event structute.
void CEditorCanvas::OnEnterWindow( wxMouseEvent& WXUNUSED(event) )
{
    SetFocus();
}


/// @brief Paint handler.
/// @param event - event structute.
void CEditorCanvas::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
    Render();
}


/// @brief Resizing handler.
/// @param event - event structute.
void CEditorCanvas::OnSize(wxSizeEvent& event)
{
    wxGLCanvas::OnSize(event);
    GetClientSize(&m_ResX, &m_ResY);
    if (GetContext())
    {
        SetCurrent();
        glViewport(0, 0, m_ResX, m_ResY);
		MatrixPerspectiveFovRH(m_mProjection, 1.0f, float(m_ResX)/float(m_ResY), 4.0f, 4500.0f, true);
    }
}


/// @brief Initialize renderer.
void CEditorCanvas::InitGL()
{
	glewInit();

    SetupCamera ();

	glClearColor(0.3f, 0.3f, 0.4f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);

	GetLight()->SetDirection(Vec4(0.0f, 0.0f, 1.0f, 0.0f));
	GetLight()->SetColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	GetLight()->Apply();
}


/// @brief Load next resource bulk.
/// @return false if finished loading.
bool CEditorCanvas::LoadNextResource()
{
    IOGResourceInfo resInfo;
	while (GetResourceMgr()->LoadNext(resInfo))
	{
		CommonToolEvent<ResLoadEventData> cmd(EVENTID_RESLOAD);
		cmd.SetEventCustomData(ResLoadEventData(wxT(resInfo.m_pResource), wxT(resInfo.m_pResourceGroup), wxT(resInfo.m_pResourceIcon)));
        GetEventHandlersTable()->FireEvent(EVENTID_RESLOAD, &cmd);
	}
	m_bLoaded = true;

	// Temporary level auto-loading
    m_pCurLevel = GetLevelManager()->LoadLevel("level_1");

	SetNewCurrentNodeForPlacement(NULL);

	return true;
}


/// @brief Level load event handler
/// @param event - event structute.
void CEditorCanvas::OnLevelLoadEvent ( CommonToolEvent<LevelLoadEventData>& event )
{
    m_pCurLevel = GetLevelManager()->LoadLevel("level_1");
	this->Refresh();
}


/// @brief Key down handler.
/// @param event - event structute.
void CEditorCanvas::OnKeyDown( wxKeyEvent& event )
{
    switch (event.GetKeyCode())
    {
    case WXK_UP:
		GetCamera()->Strafe(5.5f, Vec3(0, 0, -1));
		mouse_x = event.GetX();
		mouse_y = event.GetY();
		m_bMouseMoved = true;
		this->Refresh();
        break;

	case WXK_DOWN:
		GetCamera()->Strafe(5.5f, Vec3(0, 0, 1));
		mouse_x = event.GetX();
		mouse_y = event.GetY();
		m_bMouseMoved = true;
		this->Refresh();
        break;

	case WXK_LEFT:
		GetCamera()->Strafe(5.5f, Vec3(-1, 0, 0));
		mouse_x = event.GetX();
		mouse_y = event.GetY();
		m_bMouseMoved = true;
		this->Refresh();
        break;

	case WXK_RIGHT:
		GetCamera()->Strafe(5.5f, Vec3(1, 0, 0));
		mouse_x = event.GetX();
		mouse_y = event.GetY();
		m_bMouseMoved = true;
		this->Refresh();
        break;

    case WXK_DELETE:
        if (m_pPickedActor)
        {
            GetActorManager()->DestroyActor(m_pPickedActor);
            m_pPickedActor = NULL;
        }
		this->Refresh();
        break;

    case 'D':
        if (m_pCurNode)
        {
            m_vCurRotation.y += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.y += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'A':
        if (m_pCurNode)
        {
            m_vCurRotation.y -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.y -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'W':
        if (m_pCurNode)
        {
            m_vCurRotation.x += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.x += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'S':
        if (m_pCurNode)
        {
            m_vCurRotation.x -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.x -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'E':
        if (m_pCurNode)
        {
            m_vCurRotation.z += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.z += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'Q':
        if (m_pCurNode)
        {
            m_vCurRotation.z -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetRotation(m_vCurRotation);
        }
        if (m_pPickedActor)
        {
            Vec3 vRot = m_pPickedActor->GetSgNode()->GetRotation();
            vRot.z -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pPickedActor->GetSgNode()->SetRotation(vRot);
        }
		this->Refresh();
        break;

    case 'Z':
        if (m_pCurNode)
        {
            m_vCurScaling -= event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetScaling(m_vCurScaling);
        }
        if (m_pPickedActor)
        {
            Vec3 vScale = m_pPickedActor->GetSgNode()->GetScaling();
            vScale -= 0.02f;
            m_pPickedActor->GetSgNode()->SetScaling(vScale);
        }
		this->Refresh();
        break;

    case 'X':
        if (m_pCurNode)
        {
            m_vCurScaling += event.ControlDown() ? m_fCoarseAngleStep : m_fFineAngleStep;
            m_pCurNode->SetScaling(m_vCurScaling);
        }
        if (m_pPickedActor)
        {
            Vec3 vScale = m_pPickedActor->GetSgNode()->GetScaling();
            vScale += 0.02f;
            m_pPickedActor->GetSgNode()->SetScaling(vScale);
        }
		this->Refresh();
        break;

    case 'C':
        if (m_pCurNode)
        {
            m_vCurScaling.x = m_vCurScaling.y = m_vCurScaling.z = 1;
            m_vCurRotation.x = m_vCurRotation.y = m_vCurRotation.z = 0;
            m_pCurNode->SetWorldTransform(m_vIntersection, m_vCurRotation, m_vCurScaling);
        }
        if (m_pPickedActor)
        {
            Vec3 vPos = m_pPickedActor->GetSgNode()->GetPosition();
            m_pPickedActor->GetSgNode()->SetWorldTransform(vPos, Vec3(0, 0, 0), Vec3(1, 1, 1));
        }
		this->Refresh();
        break;
    }
    event.Skip();
}


/// @brief Key up handler.
/// @param event - event structute.
void CEditorCanvas::OnKeyUp( wxKeyEvent& event )
{
    event.Skip();
}


/// @brief Tool command event handler
/// @param event - event structute.
void CEditorCanvas::OnToolCmdEvent ( CommonToolEvent<ToolCmdEventData>& event )
{
	const ToolCmdEventData& evtData = event.GetEventCustomData();
	switch (evtData.m_CmdType)
	{
	case CMD_AABB:
		m_bShowAABB = evtData.m_bSwitcher;
		break;

	case CMD_EDITMODE_OBJECTS:
        m_pPickedActor = NULL;
        m_vCurRotation = Vec3(0, 0, 0);
        m_vCurScaling = Vec3(1, 1, 1);
		if (m_CurModelAlias.empty())
			SetNewCurrentNodeForPlacement(NULL);
		else
			SetNewCurrentNodeForPlacement(m_CurModelAlias.c_str());
		break;

	case CMD_EDITMODE_ADJUST:
        m_vCurRotation = Vec3(0, 0, 0);
        m_vCurScaling = Vec3(1, 1, 1);
        m_pPickedActor = NULL;
		SetNewCurrentNodeForPlacement(NULL);
		break;

	case CMD_EDITMODE_SETTINGS:
        m_vCurRotation = Vec3(0, 0, 0);
        m_vCurScaling = Vec3(1, 1, 1);
        m_pPickedActor = NULL;
		SetNewCurrentNodeForPlacement(NULL);
		break;
	}
	Refresh ();
}


/// @brief Setup camera.
void CEditorCanvas::SetupCamera()
{
    Vec3 vTarget (200, 0, -100);
	Vec3 vDir (0, 1.0f, 0.4f);
	vDir = vDir.normalize();
	Vec3 vUp = vDir.cross (Vec3(0, 1, 0));
	GetCamera()->Setup (vTarget + (vDir* m_fCameraDistance), vTarget, vUp);
}


/// @brief Render scene helpers.
void CEditorCanvas::RenderHelpers()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_mView.f);
    glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	if (m_bShowAABB)
	{
        if (m_pCurNode)
        {
            DrawOBB(m_pCurNode->GetOBB());
        }
	}

    if (m_pPickedActor)
    {
        DrawOBB(m_pPickedActor->GetSgNode()->GetOBB());
    }

	if (m_bIntersectionFound && m_bRedrawPatch)
	{
		DrawPatchGrid (1, &m_vIntersection);
		m_bRedrawPatch = false;
	}

    glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}


/// @brief Mouse enter handler.
/// @param event - event structute.
void CEditorCanvas::OnMouseEnter(wxMouseEvent& event)
{
	m_bMouseInWindow = true;
}


/// @brief Mouse leave handler.
/// @param event - event structute.
void CEditorCanvas::OnMouseLeave(wxMouseEvent& event)
{
	m_bMouseInWindow = false;
}


/// @brief Mouse move handler.
/// @param event - event structute.
void CEditorCanvas::OnMouseMove(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = event.GetY();
	m_bMouseMoved = true;
}


/// @brief Mouse Left button up handler.
/// @param event - event structute.
void CEditorCanvas::OnLMBUp(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = event.GetY();

    Vec3 vPick = GetPickRay (mouse_x, mouse_y);
    Vec3 vPos = GetCamera()->GetPosition();
    Vec3 vVec = vPick - vPos;
    vVec.normalize();

    ToolSettings* pTool = GetToolSettings();
    switch (pTool->GetEditMode())
    {
    case EDITMODE_OBJECTS:
        {
            if (m_pCurLevel)
            {
                m_bIntersectionFound = m_pCurLevel->GetTerrain()->GetRayIntersection(vPos, vVec, &m_vIntersection);
                if (m_bIntersectionFound)
                {
                    GetActorManager()->CreateActor(
                        OG_ACTOR_STATIC, 
                        m_CurModelAlias.c_str(), 
                        m_vIntersection, 
                        m_vCurRotation, 
                        m_vCurScaling);
                }
            }
        }
        break;

    case EDITMODE_ADJUST:
        {
            m_pPickedActor = GetActorManager()->GetNearestIntersectedActor(vPos, vVec);
        }
        break;

    case EDITMODE_SETTINGS:
        {
        }
        break;
    }

    GetActorManager()->Update(10);
    Refresh();

	bLmb = false;
}


/// @brief Mouse Left button up handler.
/// @param event - event structute.
void CEditorCanvas::OnLMBDown(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = event.GetY();
	bLmb = true;
}


/// @brief Mouse Right button up handler.
/// @param event - event structute.
void CEditorCanvas::OnRMBUp(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = event.GetY();
	bRmb = false;
}


/// @brief Mouse Right button up handler.
/// @param event - event structute.
void CEditorCanvas::OnRMBDown(wxMouseEvent& event)
{
	mouse_x = event.GetX();
	mouse_y = event.GetY();
	bRmb = true;
}


/// @brief Mouse wheel handler.
/// @param event - event structute.
void CEditorCanvas::OnMouseWheel(wxMouseEvent& event)
{
	int delta = event.GetWheelRotation();
	GetCamera()->Move ((float)delta / 10.0f);
	Refresh ();
}


/// @brief Resource switching event handler
/// @param event - event structute.
void CEditorCanvas::OnResourceSwitch ( CommonToolEvent<ResSwitchEventData>& event )
{
	const ResSwitchEventData& evtData = event.GetEventCustomData();
	switch (evtData.m_ResourceType)
	{
	case RESTYPE_MODEL:
        {
			SetNewCurrentNodeForPlacement(evtData.m_Resource);
        }
		break;
	}

	Refresh();
}


/// @brief Setup new current node for placement.
void CEditorCanvas::SetNewCurrentNodeForPlacement(const char* _pModelAlias)
{
	OG_SAFE_DELETE(m_pCurNode);
	if (_pModelAlias != NULL)
	{
		m_CurModelAlias = std::string(_pModelAlias);
		m_pCurNode = GetSceneGraph()->CreateNode(GetResourceMgr()->GetModel(_pModelAlias));
		m_pCurNode->SetWorldTransform(Vec3(0,0,0), Vec3(0,0,0), Vec3(1, 1, 1));
	}
}