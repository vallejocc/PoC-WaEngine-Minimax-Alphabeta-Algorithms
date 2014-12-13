//This module is windows compatible only!!!!!
/////////////////////////////////////////////

#ifdef TREE_DEBUG_ENABLE
#ifdef WIN32

#include "../inc/pub/wadebug.h"
#include <windows.h>
#include "resource.h"
#include <commctrl.h>
#include "../inc/pub/watypes.h"
#include "../inc/priv/wamacros.h"
#include "../inc/priv/waenginepriv.h"

unsigned int DBGON=0;
unsigned int DelayMillisecs=0;
unsigned int ContinuePressed=0;
unsigned int ContinueMode=0;
char *(*WaDebugGetDebugShowString)(WORLD_SNAPSHOT Snapshot);

//======================Handles================================================//

HINSTANCE hInst;              // main function handler
#define WIN32_LEAN_AND_MEAN   // this will assume smaller exe
TV_ITEM tvi;
HTREEITEM Selected;
TV_INSERTSTRUCT tvinsert;     // struct to config out tree control
HTREEITEM Parent;             // Tree item handle
HTREEITEM Before;             // .......
HTREEITEM Root;               // .......
HIMAGELIST hImageList;        // Image list array hadle
HBITMAP hBitMap;              // bitmap handler
int flagSelected=0;

// for drag and drop
HWND hTree;
TVHITTESTINFO tvht; 
HTREEITEM hitTarget;
POINTS Pos;
int Dragging;

// for lable editing
HWND hEdit;
HWND hDlg;

//====================MAIN DIALOG==========================================//
BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) // what are we doing ?
	{ 	 
		// This Window Message is the heart of the dialog //
		//================================================//
		case WM_INITDIALOG: 
		{
      hDlg = hWnd;

			InitCommonControls();	    // make our tree control to work

      tvinsert.hParent=NULL;			// top most level no need handle
			tvinsert.hInsertAfter=TVI_ROOT; // work as root level
      tvinsert.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
      //tvinsert.item.pszText="TREE";
			//tvinsert.item.iImage=0;
			//tvinsert.item.iSelectedImage=1;
      //Parent=(HTREEITEM)SendDlgItemMessage(hWnd,IDC_TREE1,TVM_INSERTITEM,0,(LPARAM)&tvinsert);
      Root=Parent=NULL;
      Before=Parent=NULL;
		}
		break;

		case WM_LBUTTONDOWN: 
		{
			ReleaseCapture(); 
			SendMessage(hWnd,WM_NCLBUTTONDOWN,HTCAPTION,0); 
		}
		break;

		case WM_NOTIFY:
		{
        switch(LOWORD(wParam))
        {
          case IDC_TREE1:
            
			      if(((LPNMHDR)lParam)->code == NM_DBLCLK) // if code == NM_CLICK - Single click on an item
			      {
				      char Text[255]="";
				      memset(&tvi,0,sizeof(tvi));

				      Selected=(HTREEITEM)SendDlgItemMessage(hWnd,IDC_TREE1,TVM_GETNEXTITEM,TVGN_CARET,(LPARAM)Selected);
				      
				      if(Selected==NULL)
				      {
					      MessageBox(hWnd,"No Items in TreeView","Error",MB_OK|MB_ICONINFORMATION);
					      break;
				      }
				      TreeView_EnsureVisible(hWnd,Selected);
			        SendDlgItemMessage(hWnd,IDC_TREE1,TVM_SELECTITEM,TVGN_CARET,(LPARAM)Selected);
				      flagSelected=1;
	            tvi.mask=TVIF_TEXT;
				      tvi.pszText=Text;
				      tvi.cchTextMax=256;
				      tvi.hItem=Selected;
				      
				      if(SendDlgItemMessage(hWnd,IDC_TREE1,TVM_GETITEM,TVGN_CARET,(LPARAM)&tvi))
				      {
					      if(tvi.cChildren==0 && strcmp(tvi.pszText,"Double Click Me!")==0)
					      {
					        MessageBox(hWnd,"Press OK to delete me!","Example",MB_OK|MB_ICONINFORMATION);
					        SendDlgItemMessage(hWnd,IDC_TREE1,TVM_DELETEITEM,TVGN_CARET,(LPARAM)tvi.hItem);
					        flagSelected=0;
					        break;
					      }
				      }
			      }

			      if(((LPNMHDR)lParam)->code == NM_RCLICK) // Right Click
			      {
				      Selected=(HTREEITEM)SendDlgItemMessage (hWnd,IDC_TREE1,TVM_GETNEXTITEM,TVGN_DROPHILITE,0);
				      if(Selected==NULL)
				      {
					      MessageBox(hWnd,"No Items in TreeView","Error",MB_OK|MB_ICONINFORMATION);
					      break;
				      }
				      
				      SendDlgItemMessage(hWnd,IDC_TREE1,TVM_SELECTITEM,TVGN_CARET,(LPARAM)Selected);
				      SendDlgItemMessage(hWnd,IDC_TREE1,TVM_SELECTITEM,TVGN_DROPHILITE,(LPARAM)Selected);
				      TreeView_EnsureVisible(hTree,Selected);
			      }
        }
		}
		break;

		case WM_PAINT: // constantly painting the window
		{
			return 0;
		}
		break;
		
		case WM_COMMAND: // Controling the Buttons
		{
			switch (LOWORD(wParam)) // what we pressed on?
			{ 	 

				case IDC_DELETE: // Generage Button is pressed
				{	
					if(flagSelected==1)
					{
					 if(tvi.cChildren==0)
					      SendDlgItemMessage(hWnd,IDC_TREE1,TVM_DELETEITEM,TVGN_CARET,(LPARAM)tvi.hItem);
					  flagSelected=0;
					}
					else{ 
						MessageBox(hWnd,"Double Click Item to Delete","Message",MB_OK|MB_ICONINFORMATION);
					}
				} 
				break;

				case IDC_ADDROOT:
				{
					tvinsert.hParent=NULL;			// top most level no need handle
					tvinsert.hInsertAfter=TVI_ROOT; // work as root level
					tvinsert.item.pszText="Parent Added";
					Parent=(HTREEITEM)SendDlgItemMessage(hWnd,IDC_TREE1,TVM_INSERTITEM,0,(LPARAM)&tvinsert);
					UpdateWindow(hWnd);
				}
				break;

				case IDC_CHILD:
				{
          tvinsert.hParent=Selected;			// top most level no need handle
					tvinsert.hInsertAfter=TVI_LAST; // work as root level
					tvinsert.item.pszText="Child Added";
					Parent=(HTREEITEM)SendDlgItemMessage(hWnd,IDC_TREE1,TVM_INSERTITEM,0,(LPARAM)&tvinsert);
					TreeView_SelectDropTarget(GetDlgItem(hWnd,IDC_TREE1),Parent);
				}
				break;

				case IDC_DELALL:
				{
          int i=0;
          int count = TreeView_GetCount(GetDlgItem(hWnd,IDC_TREE1));
          for(i=0;i<count;i++)
  				  TreeView_DeleteAllItems(GetDlgItem(hWnd,IDC_TREE1));
				}
				break;	

        case IDC_BUTTON5:
          ContinuePressed=1;
          break;

        case IDC_BUTTON6:
          ContinueMode=1;
          break;

        case IDC_BUTTON7:
          ContinueMode=0;
          break;

			}
			break;

			case WM_CLOSE: // We are closing the Dialog
			{
				EndDialog(hWnd,0); 
			}
			break;
		}
		break;
	}
	return 0;
}

unsigned int ThreadId;
HANDLE hThread;

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
  DialogBoxParam(((PDEBUG_CONFIG)lpParameter)->hInstance, 
                 MAKEINTRESOURCE(IDD_DIALOG1), 
                 NULL, 
                 (DLGPROC)DialogProc,0);

  return 0;
}

void DebugInit(PDEBUG_CONFIG pDbgCfg)
{ 
	DBGON=1;
  DelayMillisecs=pDbgCfg->DelayMillisecs;
  WaDebugGetDebugShowString=pDbgCfg->GetDebugShowString;

  hInst=pDbgCfg->hInstance;
  hThread=CreateThread(NULL,0,ThreadProc,pDbgCfg,0,&ThreadId);
  Sleep(3);
}

void DebugEnd()
{
  if(!DBGON)
    return;

  WaitForSingleObject(hThread,INFINITE);
}

char g_ShowStr[2000];

void RecursiveShowJob(PWORLD_STATUS pStatus,HTREEITEM hitem,unsigned int EvenOdd)
{
  unsigned int i=0;
  HTREEITEM hchilditem;
  TV_INSERTSTRUCT TreeIns;
  char * tempstr;

  for(i=0;i<pStatus->nEvolutions;i++)
  {
    tempstr=WaDebugGetDebugShowString(pStatus->SnapshotEvolutions->Snapshots[i]);
    TreeIns.hParent=hitem;			// top most level no need handle
	  TreeIns.hInsertAfter=TVI_LAST; // work as root level
    TreeIns.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
    if(EvenOdd)
    {
      TreeIns.item.state=TVIS_SELECTED;
      TreeIns.item.stateMask=TVIS_SELECTED;
    }
    else
    {
      TreeIns.item.state=0;
      TreeIns.item.stateMask=0;
    }

    itoa(pStatus->WorldStatusEvolutions[i].BestInBranch,g_ShowStr,10);
    if(tempstr)
    {
      strcat(g_ShowStr,",");
      strcat(g_ShowStr,tempstr);
    }
    TreeIns.item.pszText=g_ShowStr;
	  TreeIns.item.iImage=0;
	  TreeIns.item.iSelectedImage=0;
    hchilditem=(HTREEITEM)SendDlgItemMessage(hDlg,IDC_TREE1,TVM_INSERTITEM,0,(LPARAM)&TreeIns);
    RecursiveShowJob(&pStatus->WorldStatusEvolutions[i],hchilditem,EvenOdd^1);
  }

  TreeView_Expand(GetDlgItem(hDlg,IDC_TREE1),hitem,TVM_EXPAND);
}

void ShowJob(WORLD_ANALYSIS_JOB_HANDLE hJob)
{
  HTREEITEM hitem;
  TV_INSERTSTRUCT TreeIns;
  char * tempstr;
  PWORLD_ANALYSIS_JOB pWaJob=(PWORLD_ANALYSIS_JOB)hJob;

  if(!DBGON || !WaDebugGetDebugShowString)
    return;

	{
    int i=0;
    int count = TreeView_GetCount(GetDlgItem(hDlg,IDC_TREE1));
    for(i=0;i<count;i++)
  		TreeView_DeleteAllItems(GetDlgItem(hDlg,IDC_TREE1));
	}

  tempstr = WaDebugGetDebugShowString(pWaJob->Start->SnapShot);
  TreeIns.hParent=NULL;			// top most level no need handle
	TreeIns.hInsertAfter=TVI_ROOT; // work as root level
  TreeIns.item.mask=TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_STATE;
  TreeIns.item.state=TVIS_SELECTED;
  TreeIns.item.stateMask=TVIS_SELECTED;
  itoa(pWaJob->Start->BestInBranch,g_ShowStr,10);
  if(tempstr)
  {
    strcat(g_ShowStr,",");
    strcat(g_ShowStr,tempstr);
  }
  TreeIns.item.pszText=g_ShowStr;
	TreeIns.item.iImage=0;
	TreeIns.item.iSelectedImage=0;
  hitem=(HTREEITEM)SendDlgItemMessage(hDlg,IDC_TREE1,TVM_INSERTITEM,0,(LPARAM)&TreeIns);

  RecursiveShowJob(pWaJob->Start,hitem,0);

  if(ContinueMode || DelayMillisecs==0xffffffff)
  {
    while(!ContinuePressed);
    ContinuePressed=0;
  }
  else
  {
    Sleep(DelayMillisecs);
  }
}

#else

void DebugInit (PDEBUG_CONFIG pDbgCfg){}
void DebugEnd(){}
void ShowJob(PWORLD_ANALYSIS_JOB pWaJob){}


#endif
#endif