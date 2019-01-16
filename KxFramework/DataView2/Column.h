#pragma once
#include "Common.h"
#include "Editor.h"
#include "Renderer.h"
#include "KxFramework/KxWithOptions.h"

namespace Kx::DataView2
{
	class KX_API View;
	class KX_API MainWindow;
	class KX_API HeaderCtrl;
	class KX_API Renderer;
	class KX_API Editor;
}

namespace Kx::DataView2
{
	class KX_API Column;

	class KX_API NativeColumn: public wxSettableHeaderColumn
	{
		private:
			Column& m_Column;

		public:
			NativeColumn(Column& column)
				:m_Column(column)
			{
			}
	
		public:
			const Column& GetColumn() const
			{
				return m_Column;
			}
			Column& GetColumn()
			{
				return m_Column;
			}

		public:
			wxString GetTitle() const override;
			void SetTitle(const wxString& title) override;

			wxBitmap GetBitmap() const override;
			void SetBitmap(const wxBitmap& bitmap) override;
			
			int GetWidth() const override;
			void SetWidth(int width) override;
			
			int GetMinWidth() const override;
			void SetMinWidth(int minWidth) override;
			
			wxAlignment GetAlignment() const override;
			void SetAlignment(wxAlignment alignment) override;

			int GetFlags() const override;
			void SetFlags(int flags) override;

			bool IsSortKey() const override;
			bool IsSortOrderAscending() const override;
			void SetSortOrder(bool isAscending) override;
	};
}

namespace Kx::DataView2
{
	class KX_API Column
	{
		friend class KX_API View;
		friend class KX_API MainWindow;
		friend class KX_API HeaderCtrl;
		friend class KX_API NativeColumn;

		public:
			using Vector = std::vector<Column*>;

		private:
			static int GetAbsMinColumnWidth();
			static int GetAbsMaxColumnWidth();

		private:
			NativeColumn m_NativeColumn;
			View* m_View = nullptr;
			std::unique_ptr<Renderer> m_Renderer;
			std::unique_ptr<Editor> m_Editor;

			size_t m_Index = std::numeric_limits<size_t>::max();
			size_t m_DisplayIndex = std::numeric_limits<size_t>::max();
			int m_ID = std::numeric_limits<int>::min();

			wxBitmap m_Bitmap;
			wxString m_Title;
			wxAlignment m_TitleAlignment = wxALIGN_INVALID;

			KxWithOptions<ColumnStyle, ColumnStyle::DefaultStyle> m_Style;
			ColumnWidth m_Width;
			int m_MinWidth = 0;
			int m_BestWidth = 0;
			wxRecursionGuardFlag m_BestWidthRG;

			bool m_IsDirty = true;
			bool m_IsVisible = true;
			bool m_IsSorted = false;
			bool m_IsSortedAscending = true;

		private:
			void SetView(View* view)
			{
				m_View = view;
			}
			void SetSortOrder(bool ascending);

		protected:
			void UpdateDisplay();
			bool IsDirty() const
			{
				return m_IsDirty;
			}
			void MarkDirty(bool value = true);
			void SetIndex(size_t value)
			{
				m_Index = value;
			}

			ColumnStyle GetStyleFlags() const
			{
				return m_Style.GetOptionsValue();
			}
			void SetStyleFlags(ColumnStyle style)
			{
				m_Style.SetOptionsValue(style);
			}

			const NativeColumn& GetNativeColumn() const
			{
				return m_NativeColumn;
			}
			NativeColumn& GetNativeColumn()
			{
				return m_NativeColumn;
			}

		public:
			Column()
				:m_NativeColumn(*this)
			{
			}
			Column(const wxString& title, int id, Renderer* renderer = nullptr)
				:m_NativeColumn(*this), m_Title(title), m_Renderer(renderer), m_ID(id)
			{
			}
			Column(const wxBitmap& bitmap, int id, Renderer* renderer = nullptr)
				:m_NativeColumn(*this), m_Bitmap(bitmap), m_Renderer(renderer), m_ID(id)
			{
			}
			Column(const wxBitmap& bitmap, const wxString& title, int id, Renderer* renderer = nullptr)
				:m_NativeColumn(*this), m_Bitmap(bitmap), m_Title(title), m_Renderer(renderer), m_ID(id)
			{
			}
			
			Column(const wxString& title, int id, ColumnWidth width, ColumnStyle style)
				:m_NativeColumn(*this), m_Title(title), m_ID(id), m_Width(width), m_Style(style)
			{
			}
			Column(const wxBitmap& bitmap, int id, ColumnWidth width, ColumnStyle style)
				:m_NativeColumn(*this), m_Bitmap(bitmap), m_ID(id), m_Width(width), m_Style(style)
			{
			}

			virtual ~Column();

		public:
			View* GetView() const
			{
				return m_View;
			}
			
			Renderer& GetRenderer() const;
			void AssignRenderer(Renderer* renderer);

			Editor* GetEditor() const;
			void AssignEditor(Editor* editor);

		public:
			size_t GetIndex() const
			{
				return m_Index;
			}
			
			size_t GetDisplayIndex() const
			{
				return m_DisplayIndex;
			}
			void SetDisplayIndex(size_t value)
			{
				m_DisplayIndex = value;
			}

			int GetID() const
			{
				return m_ID;
			}
			void SetID(int id)
			{
				m_ID = id;
			}
		
			bool HasBitmap() const
			{
				return m_Bitmap.IsOk();
			}
			wxBitmap GetBitmap() const
			{
				return m_Bitmap;
			}
			void SetBitmap(const wxBitmap& bitmap)
			{
				m_Bitmap = bitmap;
				UpdateDisplay();
			}

			bool HasTitle() const
			{
				return m_Title.IsEmpty();
			}
			wxString GetTitle() const
			{
				return m_Title;
			}
			void SetTitle(const wxString& title)
			{
				m_Title = title;
				UpdateDisplay();
			}

			int GetWidth() const;
			void SetWidth(ColumnWidth width);

			int GetMinWidth() const
			{
				return m_MinWidth;
			}
			void SetMinWidth(int width)
			{
				m_MinWidth = std::clamp(width, GetAbsMinColumnWidth(), GetAbsMaxColumnWidth());
				UpdateDisplay();
			}

			bool HasBestWidth() const
			{
				return m_BestWidth > 0;
			}
			int GetBestWidth() const
			{
				return m_BestWidth;
			}
			void SetBestWidth(int width)
			{
				m_BestWidth = std::clamp(width, GetAbsMinColumnWidth(), GetAbsMaxColumnWidth());
			}
			void InvalidateBestWidth()
			{
				m_BestWidth = 0;
				MarkDirty();
			}
			int CalcBestSize();

			wxAlignment GetTitleAlignment() const
			{
				return m_TitleAlignment;
			}
			void SetTitleAlignment(wxAlignment alignment)
			{
				m_TitleAlignment = alignment;
				UpdateDisplay();
			}

			bool IsSorted() const
			{
				return m_IsSorted;
			}
			bool IsSortedAscending() const
			{
				return m_IsSortedAscending;
			}
			bool IsSortedDescending() const
			{
				return !IsSortedAscending();
			}

			void SortAscending();
			void SortDescending();
			void ToggleSortOrder();
			void ResetSorting();

			bool IsSortable()
			{
				return m_Style.IsOptionEnabled(ColumnStyle::Sort);
			}
			bool IsMoveable()
			{
				return m_Style.IsOptionEnabled(ColumnStyle::Move);
			}
			bool IsSizeable()
			{
				return m_Style.IsOptionEnabled(ColumnStyle::Size);
			}
			void SetSortable(bool value)
			{
				m_Style.SetOptionEnabled(ColumnStyle::Sort, value);
			}
			void SetMoveable(bool value)
			{
				m_Style.SetOptionEnabled(ColumnStyle::Move, value);
			}
			void SetSizeable(bool value)
			{
				m_Style.SetOptionEnabled(ColumnStyle::Size, value);
			}

			bool IsExposed(int& width) const;
			bool IsExposed() const
			{
				int width = 0;
				return IsExposed(width);
			}
			bool IsVisible() const
			{
				return m_IsVisible;
			}
			void SetVisible(bool value)
			{
				m_IsVisible = value;
			}

			bool IsRenderable() const
			{
				return m_Renderer != nullptr;
			}
			bool IsEditable() const
			{
				return m_Editor != nullptr;
			}

			bool IsExpander() const;
			bool IsHotTracked() const;
			bool IsCurrent() const;

			bool IsFirst() const;
			bool IsDisplayedFirst() const;

			bool IsLast() const;
			bool IsDisplayedLast() const;
	};
}
