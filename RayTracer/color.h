#pragma once

using color = float3;

auto const RED		= color(1.0f, 0.0f, 0.0f);
auto const GREEN	= color(0.0f, 1.0f, 0.0f);
auto const BLUE		= color(0.0f, 0.0f, 1.0f);
auto const YELLOW	= color(1.0f, 1.0f, 0.0f);
auto const PURPLE	= color(1.0f, 0.0f, 1.0f);
auto const CYAN		= color(0.0f, 1.0f, 1.0f);
auto const BLACK	= color(0.0f, 0.0f, 0.0f);
auto const WHITE	= color(1.0f, 1.0f, 1.0f);

uint constexpr RED_U	= 0xff0000;
uint constexpr GREEN_U	= 0x00ff00;
uint constexpr BLUE_U	= 0x0000ff;
uint constexpr YELLOW_U = 0xffff00;
uint constexpr PURPLE_U = 0xff00ff;
uint constexpr CYAN_U	= 0x00ffff;
uint constexpr BLACK_U	= 0x000000;
uint constexpr WHITE_U	= 0xffffff; 