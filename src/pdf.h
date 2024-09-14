#pragma once
#include "judoboard.h"
#include "../external/PDF-Writer/PDFWriter/PDFWriter.h"



namespace Judoboard
{
	class PDF
	{
	public:
		PDF(const std::string& Filename);
		~PDF();

		[[nodiscard]]
		bool IsPageOpen() const { return m_Page != nullptr;}
		void Newline() {
			double FontSize = 12;
			m_CurrentY -= FontSize * m_LineSpacing;
		}
		double GetCursorHeight() const { return m_CurrentY; }

		void CreateTable(const std::vector<std::vector<std::string>>& Data);
		void Print(const std::string& Text, double FontSize = 12, bool Bold = false);
		void PrintAt(double X, double Y, const std::string& Text, double FontSize = 12, bool Bold = false);
		
		void EndPage();


	private:
		void StartPage();
		void AddLogo();

		PDFWriter m_pdfWriter;
		PDFUsedFont* m_Font = nullptr;
		PDFUsedFont* m_FontBold = nullptr;
		PDFPage* m_Page = nullptr;
		PageContentContext* m_Content = nullptr;

		double m_CurrentX;
		double m_CurrentY;

		double m_LineSpacing = 1.5;
	};
}