#pragma once

enum class MapMusic
{
	forest,
	desert
};

enum class MusicQualityBar : unsigned int
{
	weak,
	good,
	epic,
};

enum class Drums
{
	pon, //attack
	pata, //move
	chaka,
	don,
	NONE,
};

enum class CommandState : unsigned int
{
	NONE,
	MOVE,
	ATTACK,
	//from this state (mood) not matter
	RETREAT,
	ANOTHER,
};

static CommandState commandState = CommandState::NONE;
static Drums drumPlayed = Drums::NONE;
static MusicQualityBar stageBar = MusicQualityBar::weak;