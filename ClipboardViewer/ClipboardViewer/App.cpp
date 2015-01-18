#include "./gac/GacUI.h"
#include "./gac/GacUIWindows.h"
#include <Windows.h>
#include "ClipboardUtil.h"
GuiSinglelineTextBox *formatText;
GuiSelectableButton *unicodeCheck;
GuiSelectableButton *binaryCheck;
GuiTextList *dataFormatText;
GuiMultilineTextBox *customText;
GuiMultilineTextBox *unicodeText;
GuiMultilineTextBox *htmlText;
GuiImageFrameElement *image;


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int CmdShow)
{
	return SetupWindowsDirect2DRenderer();
}
BOOL busy = FALSE;
void refreshSelectedFormat()
{
	if (busy) return;
	customText->SetText(L"");
	//GetApplication()->InvokeAsync([=]()
	{
		if (::OpenClipboard(NULL))
		{
			UINT uFormat = 0;
			char format[256] = { 0 };
			UINT htmlFormat = ::RegisterClipboardFormat(_T("HTML Format"));
			int selectFormat = wtoi(formatText->GetText());
			if (selectFormat > 0)
			{
				if (::IsClipboardFormatAvailable(selectFormat))
				{
					HGLOBAL hMem = ::GetClipboardData(selectFormat);
					if (binaryCheck->GetSelected())
					{
						LPVOID pvdata = GlobalLock(hMem);
						DWORD uDataSize = GlobalSize(hMem);
						BYTE* pbyData = new BYTE[uDataSize];
						// Copy the data to the newly-allocated memory.
						CopyMemory(pbyData, pvdata, uDataSize);
						UINT        uOffset;
						UINT        uBytesInThisLine;
						UINT        uLineOffset;

						vl::WString sLine;            // preallocate enough space for a line
						vl::WString sWork;
						vl::WString sResult;
						for (uOffset = 0; uOffset < uDataSize; uOffset += 8)
						{
							// How many bytes will be in the next line?  Max of 8.
							uBytesInThisLine = uDataSize - uOffset;
							if (uBytesInThisLine > 8)
							{
								uBytesInThisLine = 8;
							}
							// First part of the line - the starting offset.
							sLine = IntToFormatString(_T("%08X  "), uOffset);
							// Now loop through the data and add on the hex value of each byte.
							for (uLineOffset = 0; uLineOffset < uBytesInThisLine; uLineOffset++)
							{
								sWork = IntToFormatString(_T("%02X "), pbyData[uOffset + uLineOffset]);
								sLine += sWork;
								if (3 == uLineOffset || 7 == uLineOffset)
									sLine += ' ';
							}

							// If there were less than 8 bytes in this line, pad the line with
							// spaces so the ASCII representation will be in the right column.
							if (uBytesInThisLine < 8)
							{
								sLine += CharFillString(_T(' '), 1 + 3 * (8 - uBytesInThisLine));

								if (uBytesInThisLine < 4)
									sLine += _T(' ');
							}

							// Add on the ASCII representation
							for (uLineOffset = 0; uLineOffset < uBytesInThisLine; uLineOffset++)
							{
								// If the next byte isn't printable, show a period instead.
								if (isprint(pbyData[uOffset + uLineOffset]))
									sLine += (TCHAR)pbyData[uOffset + uLineOffset];
								else if (uLineOffset + 1 < uBytesInThisLine)
								{
									BYTE b[3];
									b[0] = pbyData[uOffset + uLineOffset];
									b[1] = pbyData[uOffset + uLineOffset + 1];
									b[2] = 0;
									vl::WString chinese((TCHAR *)b);
									uLineOffset++;
									sLine += chinese;
								}
								else
									sLine += _T('.');
							}
							sLine += _T('\n');
							sResult += sLine;
						}
						customText->SetText(sResult);
					}
					else if (selectFormat == CF_UNICODETEXT)
					{

						customText->SetText(customText->GetText() + (TCHAR*)GlobalLock(hMem));
					}
					else
					{
						char* data = (char*)GlobalLock(hMem);
						vl::WString unicodeData = ConvertMBCS2Utf8(data);
						if (unicodeCheck->GetSelected())
						{
							unicodeData = (TCHAR*)data;//ConvertUtf8(data);
						}
						customText->SetText(customText->GetText() + unicodeData);
					}

					GlobalUnlock(hMem);
				}
			}

			CloseClipboard();
		}
		busy = FALSE;
	}
	//);
}

void listBox_SelectionChanged(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
{
	if (dataFormatText->GetSelectedItemIndex() < 0) return;
	vl::WString selectedText = dataFormatText->GetSelectedItem()->GetText();
	int index = selectedText.IndexOf(L':');
	if (index > 0){
		formatText->SetText(selectedText.Left(index));
		refreshSelectedFormat();
	}
}

void viewButton_OnClick(GuiGraphicsComposition* sender, GuiEventArgs& arguments)
{
	dataFormatText->GetItems().Clear();
	htmlText->SetText(L"");
	unicodeText->SetText(L"");
	image->SetImage(NULL);
	if (::OpenClipboard(NULL))
	{
		UINT uFormat = 0;
		char format[256] = { 0 };
		UINT htmlFormat = ::RegisterClipboardFormat(_T("HTML Format"));
		while (uFormat = ::EnumClipboardFormats(uFormat))
		{
			vl::WString formatText = IntToString(uFormat) + L":" + GetClipFormatName(uFormat, htmlFormat);
			dataFormatText->GetItems().Add(new list::TextItem(formatText));
			if (htmlFormat == uFormat)
			{
				if (::IsClipboardFormatAvailable(uFormat))
				{
					HANDLE handle = ::GetClipboardData(uFormat);
					CHAR* data = (CHAR*)GlobalLock(handle);

					vl::WString unicodeData = ConvertUtf8(data);
					htmlText->SetText(htmlText->GetText() + unicodeData);
					GlobalUnlock(handle);
				}
			}
			else if (uFormat == CF_UNICODETEXT)
			{
				if (::IsClipboardFormatAvailable(uFormat))
				{
					HANDLE handle = ::GetClipboardData(uFormat);
					TCHAR* data = (TCHAR*)GlobalLock(handle);
					unicodeText->SetText(unicodeText->GetText() + data);
					GlobalUnlock(handle);

				}
			}
			else if (uFormat == CF_BITMAP)
			{
				if (::IsClipboardFormatAvailable(uFormat))
				{
					HBITMAP hBitmap = (HBITMAP)GetClipboardData(uFormat);
					image->SetImage(vl::presentation::windows::CreateImageFromHBITMAP(hBitmap));
				}
			}
		}
		CloseClipboard();
	}
	refreshSelectedFormat();
}

void GuiMain()
{
	GuiWindow* window = g::NewWindow();
	window->SetText(L"Clipboard Viewer");
	window->SetClientSize(Size(800, 640));
	//window->GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

	GuiTableComposition* table = new GuiTableComposition;
	table->SetRowsAndColumns(3, 1);
	table->SetRowOption(0, GuiCellOption::AbsoluteOption(30));
	table->SetRowOption(1, GuiCellOption::PercentageOption(0.4));
	table->SetRowOption(2, GuiCellOption::PercentageOption(0.6));

	// dock the table to fill the window
	table->SetAlignmentToParent(Margin(4, 4, 4, 4));
	table->SetCellPadding(6);
	// add the table to the window;
	window->GetContainerComposition()->AddChild(table);

	{
		GuiTableComposition* row1table = new GuiTableComposition;
		row1table->SetRowsAndColumns(1, 5);
		row1table->SetColumnOption(0, GuiCellOption::PercentageOption(0.2));
		row1table->SetColumnOption(1, GuiCellOption::PercentageOption(0.8));
		row1table->SetColumnOption(2, GuiCellOption::AbsoluteOption(120));
		row1table->SetColumnOption(3, GuiCellOption::AbsoluteOption(80));
		row1table->SetColumnOption(4, GuiCellOption::AbsoluteOption(80));
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			cell->SetSite(0, 0, 1, 1);
			row1table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			cell->AddChild(row1table);
		}
		{
			GuiButton *viewButton = g::NewButton();
			viewButton->SetText(L"View Clipboard");
			// dock the button to fill the cell
			viewButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			viewButton->Clicked.AttachFunction(viewButton_OnClick);
			GuiCellComposition* cell = new GuiCellComposition;
			row1table->AddChild(cell);
			cell->SetSite(0, 0, 1, 1);
			cell->AddChild(viewButton->GetBoundsComposition());
		}
		{
			GuiControl* label = g::NewLabel();
			label->SetText(L"CF_UNICODETEXT,CF_HTML,CF_BITMAP result in the bottom");
			// dock the button to fill the cell
			label->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row1table->AddChild(cell);
			cell->SetSite(0, 1, 1, 1);
			cell->AddChild(label->GetBoundsComposition());
		}
		{
			formatText = g::NewTextBox();
			formatText->SetText(L"1");
			// dock the button to fill the cell
			formatText->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row1table->AddChild(cell);
			cell->SetSite(0, 2, 1, 1);
			cell->AddChild(formatText->GetBoundsComposition());
		}
		{
			unicodeCheck = g::NewCheckBox();
			unicodeCheck->SetText(L"unicode");
			// dock the button to fill the cell
			unicodeCheck->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row1table->AddChild(cell);
			cell->SetSite(0, 3, 1, 1);
			cell->AddChild(unicodeCheck->GetBoundsComposition());
		}
		{
			binaryCheck = g::NewCheckBox();
			binaryCheck->SetText(L"binary");
			// dock the button to fill the cell
			binaryCheck->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row1table->AddChild(cell);
			cell->SetSite(0, 4, 1, 1);
			cell->AddChild(binaryCheck->GetBoundsComposition());
		}
	}
	{
		GuiTableComposition* row2table = new GuiTableComposition;
		row2table->SetRowsAndColumns(1, 2);
		row2table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
		row2table->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			// this cell is the top cell
			cell->SetSite(1, 0, 1, 1);
			row2table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			cell->AddChild(row2table);
		}
		{
			dataFormatText = g::NewTextList();
			dataFormatText->SetText(L"");
			// dock the button to fill the cell
			dataFormatText->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			dataFormatText->SelectionChanged.AttachFunction(listBox_SelectionChanged);
			GuiCellComposition* cell = new GuiCellComposition;
			row2table->AddChild(cell);
			cell->SetSite(0, 0, 1, 1);
			cell->AddChild(dataFormatText->GetBoundsComposition());
		}
		{
			customText = g::NewMultilineTextBox();
			customText->SetText(L"");
			FontProperties font;
			font.fontFamily = L"Courier New";
			font.size = 12;
			font.antialias = true;
			customText->SetFont(font);
			// dock the button to fill the cell
			customText->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row2table->AddChild(cell);
			cell->SetSite(0, 1, 1, 1);
			cell->AddChild(customText->GetBoundsComposition());
		}
	}
	{
		GuiTableComposition* row3table = new GuiTableComposition;
		row3table->SetRowsAndColumns(1, 3);
		row3table->SetColumnOption(0, GuiCellOption::PercentageOption(0.3));
		row3table->SetColumnOption(1, GuiCellOption::PercentageOption(0.3));
		row3table->SetColumnOption(2, GuiCellOption::PercentageOption(0.4));
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			cell->SetSite(2, 0, 1, 1);
			row3table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			cell->AddChild(row3table);
		}
		{
			unicodeText = g::NewMultilineTextBox();
			unicodeText->SetText(L"");
			// dock the button to fill the cell
			unicodeText->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row3table->AddChild(cell);
			cell->SetSite(0, 0, 1, 1);
			cell->AddChild(unicodeText->GetBoundsComposition());
		}
		{
			htmlText = g::NewMultilineTextBox();
			htmlText->SetText(L"");
			// dock the button to fill the cell
			htmlText->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			GuiCellComposition* cell = new GuiCellComposition;
			row3table->AddChild(cell);
			cell->SetSite(0, 1, 1, 1);
			cell->AddChild(htmlText->GetBoundsComposition());
		}
		{
			image = GuiImageFrameElement::Create();
			GuiCellComposition* cell = new GuiCellComposition;
			row3table->AddChild(cell);
			cell->SetSite(0, 2, 1, 1);
			cell->SetOwnedElement(image);
		}
	}


	// call this to calculate the size immediately if any indirect content in the table changes
	// so that the window can calcaulte its correct size before calling the MoveToScreenCenter()
	//window->ForceCalculateSizeImmediately();
	window->MoveToScreenCenter();

	GetApplication()->Run(window);
	delete window;
}