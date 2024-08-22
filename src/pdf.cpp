#include "pdf.h"
#include "../external/PDF-Writer/PDFWriter/PDFWriter.h"
#include "../external/PDF-Writer/PDFWriter/PageContentContext.h"
#include "../external/PDF-Writer/PDFWriter/PDFPage.h"
#include "../external/PDF-Writer/PDFWriter/AbstractContentContext.h"



using namespace Judoboard;



void Test()
{
	PDFWriter pdfWriter;

	// Create a new PDF file named "HelloWorld.pdf"
	if (pdfWriter.StartPDF("HelloWorld.pdf", ePDFVersion13) != PDFHummus::eSuccess)
	{
		//std::cout << "Failed to start PDF" << std::endl;
		return;
	}

	// Create a new PDF page
	PDFPage* page = new PDFPage();
	page->SetMediaBox(PDFRectangle(0, 0, 595, 842)); // A4 size page

													 // Start writing content to the page
	PageContentContext* contentContext = pdfWriter.StartPageContentContext(page);
	if (contentContext == nullptr)
	{
		//std::cout << "Failed to create content context" << std::endl;
		return;
	}

	// Draw "Hello, World!" at position (100, 700)
	contentContext->BT(); // Begin text
	contentContext->k(0, 0, 0, 1); // Set text color to black
	contentContext->Tf(pdfWriter.GetFontForFile("assets\\fonts\\arial.ttf"), 20); // Set font and size (Arial.ttf should exist)
	contentContext->Tm(1, 0, 0, 1, 100, 700); // Set text matrix (position)
	contentContext->Tj("Hello, World!"); // Set the text content
	contentContext->ET(); // End text

						  // End the page content context
	pdfWriter.EndPageContentContext(contentContext);

	// Write the page to the PDF
	pdfWriter.WritePageAndRelease(page);

	// Finish writing and close the PDF
	if (pdfWriter.EndPDF() != PDFHummus::eSuccess) {
		//std::cout << "Failed to finish PDF" << std::endl;
		return;
	}
}