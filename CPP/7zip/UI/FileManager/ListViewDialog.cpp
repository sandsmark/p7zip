// ListViewDialog.cpp

#include "StdAfx.h"

#include "../../../Windows/Clipboard.h"

#include "EditDialog.h"
#include "ListViewDialog.h"
#include "RegistryUtils.h"

#ifdef LANG
#include "LangUtils.h"
#endif

using namespace NWindows;

static const unsigned kOneStringMaxSize = 1024;


static void ListView_GetSelected(NControl::CListView &listView, CUIntVector &vector)
{
  vector.Clear();
  int index = -1;
  for (;;)
  {
    index = listView.GetNextSelectedItem(index);
    if (index < 0)
      break;
    vector.Add(index);
  }
}


bool CListViewDialog::OnInit()
{
  #ifdef LANG
  LangSetDlgItems(*this, NULL, 0);
  #endif
  _listView.Attach(GetItem(IDL_LISTVIEW));

  // FIXME if (ReadSingleClick())
    // FIXME _listView.SetExtendedListViewStyle(LVS_EX_ONECLICKACTIVATE | LVS_EX_TRACKSELECT);

  SetText(Title);

  LVCOLUMN columnInfo;
  columnInfo.mask = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
  columnInfo.fmt = LVCFMT_LEFT;
  columnInfo.iSubItem = 0;
  columnInfo.cx = 200;

  _listView.InsertColumn(0, &columnInfo);

  FOR_VECTOR (i, Strings)
    _listView.InsertItem(i, Strings[i]);

  if (Strings.Size() > 0)
    _listView.SetItemState_FocusedSelected(0);

  _listView.SetColumnWidthAuto(0);
  StringsWereChanged = false;

  NormalizeSize();
  return CModalDialog::OnInit();
}

bool CListViewDialog::OnSize(WPARAM /* wParam */, int xSize, int ySize)
{
#ifdef _WIN32
  int mx, my;
  GetMargins(8, mx, my);
  int bx1, bx2, by;
  GetItemSizes(IDCANCEL, bx1, by);
  GetItemSizes(IDOK, bx2, by);
  int y = ySize - my - by;
  int x = xSize - mx - bx1;

  /*
  RECT rect;
  GetClientRect(&rect);
  rect.top = y - my;
  InvalidateRect(&rect);
  */
  InvalidateRect(NULL);

  MoveItem(IDCANCEL, x, y, bx1, by);
  MoveItem(IDOK, x - mx - bx2, y, bx2, by);
  /*
  if (wParam == SIZE_MAXSHOW || wParam == SIZE_MAXIMIZED || wParam == SIZE_MAXHIDE)
    mx = 0;
  */
  _listView.Move(mx, my, xSize - mx * 2, y - my * 2);
#endif
  return false;
}


extern bool g_LVN_ITEMACTIVATE_Support;

void CListViewDialog::CopyToClipboard()
{
  CUIntVector indexes;
  ListView_GetSelected(_listView, indexes);
  UString s;
  
  FOR_VECTOR (i, indexes)
  {
    unsigned index = indexes[i];
    s += Strings[index];
    if (NumColumns > 1 && index < Values.Size())
    {
      const UString &v = Values[index];
      // if (!v.IsEmpty())
      {
        s += ": ";
        s += v;
      }
    }
    // if (indexes.Size() > 1)
    {
      s +=
        #ifdef _WIN32
          "\r\n"
        #else
          "\n"
        #endif
        ;
    }
  }
  
  ClipboardSetText(*this, s);
}


void CListViewDialog::ShowItemInfo()
{
  CUIntVector indexes;
  ListView_GetSelected(_listView, indexes);
  if (indexes.Size() != 1)
    return;
  unsigned index = indexes[0];

  CEditDialog dlg;
  if (NumColumns == 1)
    dlg.Text = Strings[index];
  else
  {
    dlg.Title = Strings[index];
    if (index < Values.Size())
      dlg.Text = Values[index];
  }
  
  #ifdef _WIN32
  if (dlg.Text.Find(L'\r') < 0)
    dlg.Text.Replace(L"\n", L"\r\n");
  #endif

  dlg.Create(*this);
}


void CListViewDialog::DeleteItems()
{
  for (;;)
  {
    int index = _listView.GetNextSelectedItem(-1);
    if (index < 0)
      break;
    StringsWereChanged = true;
    _listView.DeleteItem(index);
    if ((unsigned)index < Strings.Size())
      Strings.Delete(index);
    if ((unsigned)index < Values.Size())
      Values.Delete(index);
  }
  int focusedIndex = _listView.GetFocusedItem();
  if (focusedIndex >= 0)
    _listView.SetItemState_FocusedSelected(focusedIndex);
  _listView.SetColumnWidthAuto(0);
}


void CListViewDialog::OnEnter()
{
  if (IsKeyDown(VK_MENU)
      || NumColumns > 1)
  {
    ShowItemInfo();
    return;
  }
  OnOK();
}

bool CListViewDialog::OnNotify(UINT /* controlID */, LPNMHDR header)
{
#ifdef _WIN32
  if (header->hwndFrom != _listView)
    return false;
  switch (header->code)
  {
    case LVN_ITEMACTIVATE:
      if (g_LVN_ITEMACTIVATE_Support)
      {
        OnEnter();
        return true;
      }
      break;
    case NM_DBLCLK:
    case NM_RETURN: // probabably it's unused
      if (!g_LVN_ITEMACTIVATE_Support)
      {
        OnEnter();
        return true;
      }
      break;

    case LVN_KEYDOWN:
    {
      LPNMLVKEYDOWN keyDownInfo = LPNMLVKEYDOWN(header);
      switch (keyDownInfo->wVKey)
      {
        case VK_DELETE:
        {
          if (!DeleteIsAllowed)
            return false;
          DeleteItems();
          return true;
        }
        case 'A':
        {
          if (IsKeyDown(VK_CONTROL))
          {
            _listView.SelectAll();
            return true;
          }
          break;
        }
        case VK_INSERT:
        case 'C':
        {
          if (IsKeyDown(VK_CONTROL))
          {
            CopyToClipboard();
            return true;
          }
          break;
        }
      }
    }
  }
#endif
  return false;
}

void CListViewDialog::OnOK()
{
  FocusedItemIndex = _listView.GetFocusedItem();
  CModalDialog::OnOK();
}
