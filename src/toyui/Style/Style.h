//  Copyright (c) 2016 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#ifndef TOY_STYLE_H
#define TOY_STYLE_H

/* toy */
#include <toyobj/Type.h>
#include <toyobj/Indexer.h>
#include <toyobj/Util/Colour.h>
#include <toyobj/Util/NonCopy.h>
#include <toyui/Forward.h>
#include <toyui/Style/Dim.h>
#include <toyui/Style/ImageSkin.h>

/* std */
#include <vector>

namespace toy
{
	using CustomRenderer = std::function<bool (const Frame&, Renderer&)>;

	class _refl_ TOY_UI_EXPORT Shadow : public Struct
	{
	public:
		Shadow(float xpos, float ypos, float blur, float spread, Colour colour = Colour::AlphaBlack)
			: Struct()
			, d_xpos(xpos), d_ypos(ypos), d_blur(blur), d_spread(spread), d_radius(spread + blur), d_colour(colour), d_null(false)
		{}

		Shadow()
			: Struct()
			, d_null(true)
		{}

		float d_xpos;
		float d_ypos;
		float d_blur;
		float d_spread;
		float d_radius;
		Colour d_colour;
		bool d_null;

		static Type& cls() { static Type ty; return ty; }
	};

	class _refl_ TOY_UI_EXPORT Paint : public Struct
	{
	public:
		Paint() : Struct() {}
		Paint(const Colour& colour) : Struct(), m_colour(colour) {}
		Paint(const Colour& first, const Colour& second) : Struct(), m_colour(), m_gradient{ first, second } {}

		bool m_empty;
		float m_width;
		Colour m_colour;
		std::vector<Colour> m_gradient;
		Image* m_image;

		static Type& cls() { static Type ty; return ty; }
	};

	template <class T>
	class StyleAttr
	{
	public:
		StyleAttr() : val(), set(false) {}
		StyleAttr(const T& v) : val(v), set(false) {}
		StyleAttr(const StyleAttr& other) : val(other.val), set(other.set) {}

		operator const T&() const { return val; }
		operator T&() { return val; }

		template <class U>
		StyleAttr& operator=(const U& v) { val = v; set = true; return *this; }
		StyleAttr& operator=(const T& v) { val = v; set = true; return *this; }

		void copy(const StyleAttr& other, bool inherit) { if(set && (inherit || !other.set)) return; val = other.val; if(!inherit) set = other.set; }

		T val;
		bool set;
	};

	class _refl_ TOY_UI_EXPORT LayoutStyle : public Struct
	{
	public:
		LayoutStyle()
			: Struct()
			, d_solver(FRAME_SOLVER) , d_layout(DimLayout(AUTO_LAYOUT, AUTO_LAYOUT)), d_flow(FLOW), d_space(Space::preset(SHEET))
			, d_clipping(NOCLIP), d_opacity(CLEAR), d_align(DimAlign(LEFT, LEFT))
			, d_span(DimFloat(1.f, 1.f)), d_pivot(DimPivot(FORWARD, FORWARD)), d_zorder(0), d_updated(0)
		{}

		LayoutStyle(const LayoutStyle& other)
			: Struct()
		{
			this->copy(other);
		}

		void copy(const LayoutStyle& other, bool inherit)
		{
			d_solver.copy(other.d_solver, inherit);
			d_layout.copy(other.d_layout, inherit);
			d_flow.copy(other.d_flow, inherit);
			d_space.copy(other.d_space, inherit);
			d_clipping.copy(other.d_clipping, inherit);
			d_opacity.copy(other.d_opacity, inherit);
			d_align.copy(other.d_align, inherit);
			d_span.copy(other.d_span, inherit);
			d_size.copy(other.d_size, inherit);
			d_padding.copy(other.d_padding, inherit);
			d_margin.copy(other.d_margin, inherit);
			d_spacing.copy(other.d_spacing, inherit);
			d_pivot.copy(other.d_pivot, inherit);
			d_zorder.copy(other.d_zorder, inherit);
		}

		void inherit(const LayoutStyle& other) { return this->copy(other, true); }
		void copy(const LayoutStyle& other) { return this->copy(other, false); }

		StyleAttr<LayoutSolver> d_solver;
		StyleAttr<DimLayout> d_layout;
		StyleAttr<Flow> d_flow;
		StyleAttr<Space> d_space;
		StyleAttr<Clipping> d_clipping;
		StyleAttr<Opacity> d_opacity;
		StyleAttr<DimAlign> d_align;
		StyleAttr<DimFloat> d_span;
		StyleAttr<DimFloat> d_size;
		StyleAttr<BoxFloat> d_padding;
		StyleAttr<DimFloat> d_margin;
		StyleAttr<DimFloat> d_spacing;
		StyleAttr<DimPivot> d_pivot;
		StyleAttr<int> d_zorder;

		_attr_ _mut_ size_t d_updated;

		static Type& cls() { static Type ty; return ty; }
	};

	class _refl_ TOY_UI_EXPORT InkStyle : public Struct
	{
	public:
		_constr_ InkStyle(Style* style = nullptr)
			: Struct()
			, m_style(style)
			, m_empty(true), m_base(nullptr), m_backgroundColour(Colour::None), m_borderColour(Colour::None), m_imageColour(Colour::None), m_textColour(Colour::None)
			, m_textFont("dejavu"), m_textSize(14.f), m_textBreak(true), m_textWrap(false)
			, m_borderWidth(0.f), m_cornerRadius(), m_weakCorners(false)
			, m_padding(0.f), m_margin(0.f)
			, m_align(DimAlign(LEFT, LEFT)), m_linearGradient(DimFloat(0.f, 0.f)), m_linearGradientDim(DIM_Y)
			, m_image(nullptr), m_overlay(nullptr), m_tile(nullptr), m_hoverCursor(nullptr)
		{}

		InkStyle(const InkStyle& other)
			: Struct()
			, m_style(other.m_style)
		{
			this->copy(other);
		}

		void copy(const InkStyle& other, bool inherit)
		{
			if(!inherit)
				m_base.copy(other.m_base, false);
			if(inherit && m_base.set && other.m_style != m_base.val)
				return;

			m_empty.copy(other.m_empty, inherit);
			m_backgroundColour.copy(other.m_backgroundColour, inherit);
			m_borderColour.copy(other.m_borderColour, inherit);
			m_imageColour.copy(other.m_imageColour, inherit);
			m_textColour.copy(other.m_textColour, inherit);
			m_textFont.copy(other.m_textFont, inherit);
			m_textSize.copy(other.m_textSize, inherit);
			m_textBreak.copy(other.m_textBreak, inherit);
			m_textWrap.copy(other.m_textWrap, inherit);
			m_borderWidth.copy(other.m_borderWidth, inherit);
			m_cornerRadius.copy(other.m_cornerRadius, inherit);
			m_weakCorners.copy(other.m_weakCorners, inherit);
			m_padding.copy(other.m_padding, inherit);
			m_margin.copy(other.m_margin, inherit);
			m_align.copy(other.m_align, inherit);
			m_linearGradient.copy(other.m_linearGradient, inherit);
			m_linearGradientDim.copy(other.m_linearGradientDim, inherit);
			m_image.copy(other.m_image, inherit);
			m_overlay.copy(other.m_overlay, inherit);
			m_tile.copy(other.m_tile, inherit);
			m_imageSkin.copy(other.m_imageSkin, inherit);
			m_shadow.copy(other.m_shadow, inherit);
			m_hoverCursor.copy(other.m_hoverCursor, inherit);
			m_customRenderer.copy(other.m_customRenderer, inherit);
		}

		void inherit(const InkStyle& other) { return this->copy(other, true); }
		void copy(const InkStyle& other) { return this->copy(other, false); }

		void setEmpty(bool empty) { m_empty = empty; }

		void prepare();

		Style* m_style;
		StyleAttr<bool> m_empty;
		StyleAttr<Style*> m_base;
		StyleAttr<Colour> m_backgroundColour;
		StyleAttr<Colour> m_borderColour;
		StyleAttr<Colour> m_imageColour;
		StyleAttr<Colour> m_textColour;
		StyleAttr<string> m_textFont;
		StyleAttr<float> m_textSize;
		StyleAttr<bool> m_textBreak;
		StyleAttr<bool> m_textWrap;
		StyleAttr<BoxFloat> m_borderWidth;
		StyleAttr<BoxFloat> m_cornerRadius;
		StyleAttr<bool> m_weakCorners;
		StyleAttr<BoxFloat> m_padding;
		StyleAttr<BoxFloat> m_margin;
		StyleAttr<DimAlign> m_align;
		StyleAttr<DimFloat> m_linearGradient;
		StyleAttr<Dimension> m_linearGradientDim;
		StyleAttr<Image*> m_image;
		StyleAttr<Image*> m_overlay;
		StyleAttr<Image*> m_tile;
		StyleAttr<ImageSkin> m_imageSkin;
		StyleAttr<Shadow> m_shadow;
		StyleAttr<Type*> m_hoverCursor;
		StyleAttr<CustomRenderer> m_customRenderer;

		static Type& cls() { static Type ty; return ty; }
	};

	class TOY_UI_EXPORT SubSkin
	{
	public:
		SubSkin(WidgetState state, Style& style) : m_state(state), m_skin(&style) {}
		SubSkin(WidgetState state, const InkStyle& skin) : m_state(state), m_skin(skin) {}

		WidgetState m_state;
		InkStyle m_skin;
	};

	typedef std::vector<SubSkin> StyleTable;

	class _refl_ TOY_UI_EXPORT Style : public IdObject
	{
	public:
		Style(Type& type, Style* base = nullptr);
		Style(const string& name);
		~Style();

		Style& operator=(const Style&) = default;

		const string& name() { return m_name.empty() ? m_style->name() : m_name; }
		
		void clear();
		void prepare(Style* definition);

		InkStyle& subskin(WidgetState state);
		InkStyle& decline(WidgetState state);

		InkStyle& fetchSubskin(WidgetState state);

		void inheritLayout(Style& base);
		void inheritSkin(Style& base);

		void define(Style& style);

		void copyLayout(Style& base);
		void copySkin(Style& base);

		static Type& cls() { static Type ty(INDEXED); return ty; }

	public:
		Type* m_style;
		_attr_ Style* m_base;
		_attr_ string m_name;
		_attr_ LayoutStyle m_layout;
		_attr_ InkStyle m_skin;
		StyleTable m_subskins;

		bool m_ready;
	};
}

#endif // TOY_STYLE_H
