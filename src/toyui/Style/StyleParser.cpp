//  Copyright (c) 2016 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#include <toyui/Config.h>
#include <toyui/Style/StyleParser.h>

#include <toyobj/String/StringConvert.h>

#include <toyui/Widget/Widget.h>
#include <toyui/UiLayout.h>
#include <toyui/UiWindow.h>

#include <yaml.h>

namespace toy
{
	template <> Direction fromString<Direction>(const string& str) { if(str == "READING") return READING; else if(str == "PARAGRAPH") return PARAGRAPH; return READING; }
	template <> Sizing fromString<Sizing>(const string& str) { if(str == "FIXED") return FIXED; else if(str == "SHRINK") return SHRINK; else if(str == "WRAP") return WRAP; else if(str == "EXPAND") return EXPAND; return FIXED; }

	template <> Space fromString<Space>(const string& str) { std::vector<string> elements = splitString(str, ","); return { fromString<Direction>(elements[0]), fromString<Sizing>(elements[1]), fromString<Sizing>(elements[2]) }; }

	template <> Flow fromString<Flow>(const string& str) { if(str == "FLOW") return FLOW; else if(str == "OVERLAY") return OVERLAY; return FLOW; }
	template <> Clipping fromString<Clipping>(const string& str) { if(str == "NOCLIP") return NOCLIP; else if(str == "CLIP") return CLIP; return NOCLIP; }
	template <> Opacity fromString<Opacity>(const string& str) { if(str == "OPAQUE") return OPAQUE; else if(str == "CLEAR") return CLEAR; else if(str == "HOLLOW") return HOLLOW;  return OPAQUE; }
	template <> Align fromString<Align>(const string& str) { if(str == "CENTER") return CENTER; else if(str == "LEFT") return LEFT; else if(str == "RIGHT") return RIGHT; return LEFT; }
	template <> Pivot fromString<Pivot>(const string& str) { if(str == "FORWARD") return FORWARD; else if(str == "REVERSE") return REVERSE; return FORWARD; }

	template <> inline void fromString<DimPivot>(const string& str, DimPivot& dim) { string_to_fixed_vector<DimPivot, Pivot>(str, dim); }
	template <> inline void fromString<DimAlign>(const string& str, DimAlign& dim) { string_to_fixed_vector<DimAlign, Align>(str, dim); }
	
	template <> inline void fromString<DimFloat>(const string& str, DimFloat& vec) { string_to_fixed_vector<DimFloat, float>(str, vec); }
	template <> inline void fromString<BoxFloat>(const string& str, BoxFloat& vec) { string_to_fixed_vector<BoxFloat, float>(str, vec); }

	template <> WidgetState fromString<WidgetState>(const string& str)
	{
		int state = 0;
		std::vector<string> names = splitString(str, "|");
		std::vector<WidgetState> states;
		for(const string& name : names)
		{
			if(name == "focused") state |= FOCUSED;
			else if(name == "triggered") state |= TRIGGERED;
			else if(name == "pressed") state |= TRIGGERED;
			else if(name == "activated") state |= ACTIVATED;
			else if(name == "selected") state |= SELECTED;
			else if(name == "dragged") state |= DRAGGED;
			else if(name == "disabled") state |= DISABLED;
			else if(name == "active") state |= ACTIVE;
			else if(name == "modal") state |= MODAL;
			else if(name == "control") state |= CONTROL;
		}

		return static_cast<WidgetState>(state);
	}

	class StyleParser::Impl
	{
	public:
		Impl() { yaml_parser_initialize(&m_parser); }
		~Impl() { yaml_parser_delete(&m_parser); }

		yaml_parser_t m_parser;
	};

	StyleParser::StyleParser(Styler& styler)
		: m_styler(styler)
		, m_state(IN_DOCUMENT)
		, m_keyState(IN_KEY_DEFINITION)
		, m_pimpl(make_unique<Impl>())
	{}

	StyleParser::~StyleParser()
	{}

	void StyleParser::loadDefaultStyle()
	{
		m_styler.reset();
	}

	void StyleParser::loadStyleSheet(const string& path)
	{
		m_styler.clear();

		FILE* input = fopen(path.c_str(), "rb");

		if(!input)
		{
			printf("ERROR: Could not open style sheet %s", path.c_str());
			return;
		}

		yaml_parser_set_input_file(&m_pimpl->m_parser, input);

		yaml_token_t token;
		bool done = false;
		while(!done)
		{
			if(!yaml_parser_scan(&m_pimpl->m_parser, &token))
				return;

			switch(token.type)
			{
			case YAML_KEY_TOKEN:
				m_keyState = IN_KEY_DEFINITION;
				break;
			case YAML_VALUE_TOKEN:
				m_keyState = IN_VALUE_DEFINITION;
				break;
			case YAML_BLOCK_MAPPING_START_TOKEN:
			case YAML_FLOW_MAPPING_START_TOKEN:
				if(m_state == IN_DOCUMENT)
					m_state = IN_MAIN_BLOCK;
				else if(m_state == IN_MAIN_BLOCK)
					this->startStyle(m_key);
				else if(m_state == IN_STYLE_DEFINITION)
					this->startSubskin(m_key);
				break;
			case YAML_BLOCK_END_TOKEN:
			case YAML_FLOW_MAPPING_END_TOKEN:
				if(m_state == IN_STYLE_DEFINITION)
					m_state = IN_MAIN_BLOCK;
				if(m_state == IN_SUBSKIN_DEFINITION)
					m_state = IN_STYLE_DEFINITION;
				break;
			case YAML_SCALAR_TOKEN:
				if(m_keyState == IN_KEY_DEFINITION)
					m_key = reinterpret_cast<const char*>(token.data.scalar.value);
				else if(m_keyState == IN_VALUE_DEFINITION)
					this->parseValue(m_key, reinterpret_cast<const char*>(token.data.scalar.value));
				break;
			default:
				break;
			}

			done = (token.type == YAML_STREAM_END_TOKEN);
			yaml_token_delete(&token);
		}

		m_styler.reset();
	}

	void StyleParser::startStyle(const string& name)
	{
		m_state = IN_STYLE_DEFINITION;
		m_style = &m_styler.styledef(name);
		m_skin = &m_style->m_skin;
		m_style->m_skin.m_empty = false;
	}

	void StyleParser::startSubskin(const string& name)
	{
		m_state = IN_SUBSKIN_DEFINITION;
		string clean = replaceAll(name, " ", "");
		WidgetState state = fromString<WidgetState>(clean);
		m_skin = &m_style->decline(state);
	}

	void StyleParser::declineImage(const string& strStates)
	{
		std::vector<string> states = splitString(strStates, ",");
		for(const string& strState : states)
		{
			WidgetState state = fromString<WidgetState>(strState);
			string suffix = "_" + replaceAll(strState, "|", "_");
			m_style->decline(state).m_image = &m_styler.uiWindow().findImage(m_skin->m_image.val->d_name + suffix);
		}
	}
	
	void StyleParser::declineImageSkin(const string& strStates)
	{
		std::vector<string> states = splitString(strStates, ",");
		for(const string& strState : states)
		{
			WidgetState state = fromString<WidgetState>(strState);
			string suffix = "_" + replaceAll(strState, "|", "_");
			InkStyle& inkstyle = m_style->decline(state);
			inkstyle.m_imageSkin = ImageSkin(m_styler.uiWindow().findImage(m_skin->m_imageSkin.val.d_image->d_name + suffix), m_skin->m_imageSkin);
		}
	}

	void StyleParser::parseValue(const string& key, const string& valueRaw)
	{
		string value = replaceAll(valueRaw, " ", "");
		std::vector<string> values = splitString(value, ",");
		
		if(key == "copy_skin")
			m_style->copySkin(m_styler.styledef(value));
		//else if(key == "inherit_skin")
		//	m_style->inheritSkin(m_styler.styledef(value));
		else if(key == "reset_skin")
			m_style->m_skin.m_base = nullptr;

		else if(key == "flow")
			m_style->m_layout.d_flow = fromString<Flow>(value); // FLOW | OVERLAY | FLOAT
		else if(key == "clipping")
			m_style->m_layout.d_clipping = fromString<Clipping>(value); // NOCLIP | CLIP
		else if(key == "opacity")
			m_style->m_layout.d_opacity = fromString<Opacity>(value); // OPAQUE | CLEAR | HOLLOW
		else if(key == "space")
			m_style->m_layout.d_space = fromString<Space>(value);
		else if(key == "align")
			m_style->m_layout.d_align = fromString<DimAlign>(value); // x, y
		else if(key == "span")
			m_style->m_layout.d_span = fromString<DimFloat>(value); // 1.0, 1.0
		else if(key == "size")
			m_style->m_layout.d_size = fromString<DimFloat>(value); // 123.0, 123.0
		else if(key == "padding")
			m_style->m_layout.d_padding = fromString<BoxFloat>(value); // left, right, top, bottom
		else if(key == "margin")
			m_style->m_layout.d_margin = fromString<DimFloat>(value); // x, y
		else if(key == "spacing")
			m_style->m_layout.d_spacing = fromString<DimFloat>(value); // x, y
		else if(key == "pivot")
			m_style->m_layout.d_pivot = fromString<DimPivot>(value);// FORWARD | REVERSE

		else if(key == "empty")
			m_skin->m_empty = (value == "false" ? false : true);
		else if(key == "background_colour")
			m_skin->m_backgroundColour = fromString<Colour>(value); // r, g, b, a
		else if(key == "border_colour")
			m_skin->m_borderColour = fromString<Colour>(value); // r, g, b, a
		else if(key == "image_colour")
			m_skin->m_imageColour = fromString<Colour>(value); // r, g, b, a
		else if(key == "text_colour")
			m_skin->m_textColour = fromString<Colour>(value); // r, g, b, a
		else if(key == "text_size")
			m_skin->m_textSize = fromString<float>(value); // 0.0
		else if(key == "text_colour")
			m_skin->m_textFont = value; // fontname
		else if(key == "border_width")
			m_skin->m_borderWidth = fromString<BoxFloat>(value); // top, right, bottom, left
		else if(key == "corner_radius")
			m_skin->m_cornerRadius = fromString<BoxFloat>(value); // topleft, topright, bottomright, bottomleft
		else if(key == "weak_corners")
			m_skin->m_weakCorners = (value == "false" ? false : true); // true | false
		else if(key == "skin_align")
			m_skin->m_align = fromString<DimAlign>(value); // x, y
		else if(key == "skin_padding")
			m_skin->m_padding = fromString<BoxFloat>(value); // left, right, top, bottom
		else if(key == "skin_margin")
			m_skin->m_margin = fromString<BoxFloat>(value); // x, y, z, w
		else if(key == "topdown_gradient")
			m_skin->m_linearGradient = fromString<DimFloat>(value); // top, down
		else if(key == "image")
			m_skin->m_image = value == "null" ? nullptr : &m_styler.uiWindow().findImage(value); // image.png
		else if(key == "overlay")
			m_skin->m_overlay = value == "null" ? nullptr : &m_styler.uiWindow().findImage(value); // image.png
		else if(key == "tile")
			m_skin->m_tile = value == "null" ? nullptr : &m_styler.uiWindow().findImage(value); // image.png
		else if(key == "image_skin")
			m_skin->m_imageSkin = ImageSkin(m_styler.uiWindow().findImage(values[0]),
											fromString<int>(values[1]), fromString<int>(values[2]),
											fromString<int>(values[3]), fromString<int>(values[4]),
											values.size() > 5 ? fromString<int>(values[5]) : 0,
											values.size() > 6 ? fromString<Dimension>(values[6]) : DIM_NULL); // : image.png
		else if(key == "shadow")
			m_skin->m_shadow = Shadow(fromString<float>(values[0]), fromString<float>(values[1]),
									  fromString<float>(values[2]), fromString<float>(values[3])); // : xoffset, yoffset, blur, spread
		else if(key == "no_shadow")
			m_skin->m_shadow = Shadow();
		else if(key == "shadow_colour")
			m_skin->m_shadow.val.d_colour = fromString<Colour>(value);
		else if(key == "decline_image")
			this->declineImage(value);
		else if(key == "decline_image_skin")
			this->declineImageSkin(value);
	}
}
