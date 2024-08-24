#include "pdf.h"
#include "../external/license/license.h"
#include "../external/PDF-Writer/PDFWriter/PageContentContext.h"
#include "../external/PDF-Writer/PDFWriter/PDFPage.h"
#include "../external/PDF-Writer/PDFWriter/AbstractContentContext.h"



using namespace Judoboard;



PDF::PDF(const std::string& Filename)
{
	//Create a new PDF file
	if (m_pdfWriter.StartPDF(Filename, ePDFVersion13) != PDFHummus::eSuccess)
	{
		//std::cout << "Failed to start PDF" << std::endl;
		return;
	}

	m_Font = m_pdfWriter.GetFontForFile("assets\\fonts\\arial.ttf");
	m_FontBold = m_pdfWriter.GetFontForFile("assets\\fonts\\arial bold.ttf");
}



PDF::~PDF()
{
	if (IsPageOpen())
		EndPage();

	//Finish writing and close the PDF
	if (m_pdfWriter.EndPDF() != PDFHummus::eSuccess)
	{
		//std::cout << "Failed to finish PDF" << std::endl;
		return;
	}
}



void PDF::StartPage()
{
	if (IsPageOpen())
		EndPage();

	//Create a new PDF page
	m_Page = new PDFPage();
	m_Page->SetMediaBox(PDFRectangle(0, 0, 595, 842));//A4 size page, size in points, 1 point = 1/72 of an inch

	//Start writing content to the page
	m_Content = m_pdfWriter.StartPageContentContext(m_Page);
	if (!m_Content)
	{
		//std::cout << "Failed to create content context" << std::endl;
		return;
	}

	AddLogo();

	m_CurrentX = 30;
	m_CurrentY = 810;
}



void PDF::EndPage()
{
	if (IsPageOpen())
	{
		//End the page content
		m_pdfWriter.EndPageContentContext(m_Content);

		//Write the page to the PDF
		m_pdfWriter.WritePageAndRelease(m_Page);

		m_Content = nullptr;
		m_Page    = nullptr;
	}

	assert(!IsPageOpen());
}



void PDF::CreateTable(const std::vector<std::vector<std::string>>& Data)
{
	if (!IsPageOpen())
		StartPage();

	//Table parameters
	const int rows = (int)Data.size();
	const int cols = 5;
	const double startX = m_CurrentX;//Starting X position for the table
	const double startY = m_CurrentY;//Starting Y position for the table

	const double cellWidth[] = { 23, 210, 205, 43, 45 };
	const double cellHeight = 30;//Height of each cell

	double totalCellWidth = 0.0;
	for (auto width : cellWidth)
		totalCellWidth += width;

	//1. Draw horizontal lines
	for (int i = 0; i <= rows; ++i)
	{
		m_Content->q();//Save graphic state
		m_Content->m(startX, startY - i * cellHeight);//Move to starting point of the line
		m_Content->l(startX + totalCellWidth, startY - i * cellHeight);//Line to the end point
		m_Content->S();//Stroke the line
		m_Content->Q();//Restore graphic state
	}

	//Draw vertical lines (columns)
	double x = startX; 
	for (int j = 0; j <= cols; ++j)
	{
		m_Content->q();//Save graphic state
		m_Content->m(x, startY);//Move to starting point of the line
		m_Content->l(x, startY - rows * cellHeight);//Line to the end point
		m_Content->S();//Stroke the line
		m_Content->Q();//Restore graphic state

		x += cellWidth[j];
	}

	//2. Add text inside the table cells
	for (int i = 0; i < rows; ++i)
	{
		double x = startX;

		for (int j = 0; j < cols; ++j)
		{
			//Example text: R1C1, R1C2...
			//std::string text = "R" + std::to_string(i+1) + "C" + std::to_string(j+1);
			std::string text = Data[i][j];

			//Set position for the text in each cell (centered)
			double textX = x + 5;//A small offset to align text better
			double textY = startY - i * cellHeight - 20;//Center vertically in the cell

			//Set text color to black and place text
			m_Content->k(0, 0, 0, 1);//Black color
			m_Content->BT();//Begin Text
			m_Content->Tf(m_Font, 12);//Set the font and size (only needs to be set per page, not reloaded)
			m_Content->Tm(1, 0, 0, 1, textX, textY);//Set position for the text
			m_Content->Tj(text.c_str()); // Show text
			m_Content->ET();//End Text

			x += cellWidth[j];
		}
	}

	m_CurrentY -= cellHeight * rows + 12 * m_LineSpacing;
}



void PDF::PrintAt(double X, double Y, const std::string& Text, double FontSize, bool Bold)
{
	m_Content->BT();//Begin text
	m_Content->k(0, 0, 0, 1);//Set text color to black (CMYK)

	if (Bold)
		m_Content->Tf(m_FontBold, FontSize);//Set font and size
	else
		m_Content->Tf(m_Font, FontSize);//Set font and size

	m_Content->Tm(1, 0, 0, 1, X, Y);//Set text matrix (position), m_CurrentX to right, m_CurrentY from bottom
	m_Content->Tj(Text);//Set the text content
	m_Content->ET();//End text
}



void PDF::Print(const std::string& Text, double FontSize, bool Bold)
{
	if (!IsPageOpen())
		StartPage();

	//Draw text at position current position
	PrintAt(m_CurrentX, m_CurrentY, Text, FontSize, Bold);

	m_CurrentY -= FontSize * m_LineSpacing;
}



void PDF::AddLogo()
{
	//Define page dimensions
	double pageWidth  = 595;//A4 width in points
	double pageHeight = 842;//A4 height in points

	//Calculate position for top right corner
	double x = pageWidth  - 100 - 25;//margin from the right edge
	double y = pageHeight - 30;//margin from the top edge

	// Draw the image
	AbstractContentContext::ImageOptions opt;
	//opt.transformationMethod = AbstractContentContext::eMatrix;
	//opt.matrix[0] = opt.matrix[3] = 0.25;
	opt.transformationMethod = AbstractContentContext::eFit;
	opt.boundingBoxWidth  = 113;
	opt.boundingBoxHeight = 100;
	opt.fitProportional   = true;
	m_Content->DrawImage(x, y, "assets\\logo.png", opt);

	std::string license = License::GetUserID();
	PrintAt(pageWidth - 100 - 25, pageHeight - 30 - 10, "Lizenz: " + license, 10);
}