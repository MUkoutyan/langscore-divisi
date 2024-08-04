const chai = require('chai');
const sinon = require('sinon');
const expect = chai.expect;
const path = require('path');
const fs = require('fs');
const fontkit = require('fontkit');
const fetch = require('node-fetch');
const { before } = require('mocha');
//MZのみの対応
const { Worker, isMainThread, parentPort, workerData } = require('worker_threads');

const { JSDOM } = require("jsdom");
const { createCanvas } = require('canvas');

global.FontFace = class {
  constructor(family, source) {
    this.family = family;
    this.source = source;
    this.status = 'unloaded';
  }

  async load() {
    try {
      const response = await fetch(this.source.slice(4, -1)); // Remove "url(" and ")"
      const arrayBuffer = await response.arrayBuffer();
      const font = fontkit.create(arrayBuffer);
      this.status = 'loaded';
      return font;
    } catch (error) {
      this.status = 'error';
      throw error;
    }
  }
};

// START OF GENERATED CONTENT
const testActors = [{'ja': 'エルーシェ', 'en': 'eluche'}, {'ja': '雑用係', 'en': 'Compassionate'}, {'ja': 'ラフィーナ', 'en': 'Rafina'}, {'ja': '傲慢ちき', 'en': 'arrogant'}, {'ja': 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en': "Kestinius' scullery maid.\nThere is not that much work."}, {'ja': 'チビのツンデレウーマン。\n魔法が得意。', 'en': 'Tiny tsundere woman.\nHe is good at magic.'}];
const testArmors = [{'ja': '盾', 'en': 'Shield'}, {'ja': '帽子', 'en': 'Had'}, {'ja': '服', 'en': 'Wear'}, {'ja': '指輪', 'en': 'Ring'}];
const testClasses = [{'ja': '勇者', 'en': 'brave'}, {'ja': '戦士', 'en': 'warrior'}, {'ja': '魔術師', 'en': 'magician'}, {'ja': '僧侶', 'en': 'monk'}];
const testCommonevents = [{'ja': '顧問です', 'en': 'Advisor.'}, {'ja': ' ', 'en': ''}];
const testEnemies = [{'ja': 'こうもり', 'en': 'umbrella'}, {'ja': 'スライム', 'en': 'slime'}, {'ja': 'オーク', 'en': 'oak'}, {'ja': 'ミノタウロス', 'en': 'Minotaur'}];
const testGraphics = [{'ja': 'img/animations/Absorb', 'en': ''}, {'ja': 'img/animations/ArrowSpecial', 'en': ''}, {'ja': 'img/animations/Blow', 'en': ''}, {'ja': 'img/animations/Breath', 'en': ''}, {'ja': 'img/animations/Claw', 'en': ''}, {'ja': 'img/animations/ClawPhoton', 'en': ''}, {'ja': 'img/animations/ClawSpecial1', 'en': ''}, {'ja': 'img/animations/ClawSpecial2', 'en': ''}, {'ja': 'img/animations/Cure1', 'en': ''}, {'ja': 'img/animations/Cure2', 'en': ''}, {'ja': 'img/animations/Cure3', 'en': ''}, {'ja': 'img/animations/Cure4', 'en': ''}, {'ja': 'img/animations/Curse', 'en': ''}, {'ja': 'img/animations/Darkness1', 'en': ''}, {'ja': 'img/animations/Darkness2', 'en': ''}, {'ja': 'img/animations/Darkness3', 'en': ''}, {'ja': 'img/animations/Darkness4', 'en': ''}, {'ja': 'img/animations/Darkness5', 'en': ''}, {'ja': 'img/animations/Earth1', 'en': ''}, {'ja': 'img/animations/Earth2', 'en': ''}, {'ja': 'img/animations/Earth3', 'en': ''}, {'ja': 'img/animations/Earth4', 'en': ''}, {'ja': 'img/animations/Earth5', 'en': ''}, {'ja': 'img/animations/Explosion1', 'en': ''}, {'ja': 'img/animations/Explosion2', 'en': ''}, {'ja': 'img/animations/Fire1', 'en': ''}, {'ja': 'img/animations/Fire2', 'en': ''}, {'ja': 'img/animations/Fire3', 'en': ''}, {'ja': 'img/animations/Flash', 'en': ''}, {'ja': 'img/animations/Gun1', 'en': ''}, {'ja': 'img/animations/Gun2', 'en': ''}, {'ja': 'img/animations/Gun3', 'en': ''}, {'ja': 'img/animations/Hit1', 'en': ''}, {'ja': 'img/animations/Hit2', 'en': ''}, {'ja': 'img/animations/HitFire', 'en': ''}, {'ja': 'img/animations/HitIce', 'en': ''}, {'ja': 'img/animations/HitPhoton', 'en': ''}, {'ja': 'img/animations/HitSpecial1', 'en': ''}, {'ja': 'img/animations/HitSpecial2', 'en': ''}, {'ja': 'img/animations/HitThunder', 'en': ''}, {'ja': 'img/animations/Holy1', 'en': ''}, {'ja': 'img/animations/Holy2', 'en': ''}, {'ja': 'img/animations/Holy3', 'en': ''}, {'ja': 'img/animations/Holy4', 'en': ''}, {'ja': 'img/animations/Holy5', 'en': ''}, {'ja': 'img/animations/Howl', 'en': ''}, {'ja': 'img/animations/Ice1', 'en': ''}, {'ja': 'img/animations/Ice2', 'en': ''}, {'ja': 'img/animations/Ice3', 'en': ''}, {'ja': 'img/animations/Ice4', 'en': ''}, {'ja': 'img/animations/Ice5', 'en': ''}, {'ja': 'img/animations/Laser1', 'en': ''}, {'ja': 'img/animations/Laser2', 'en': ''}, {'ja': 'img/animations/Light1', 'en': ''}, {'ja': 'img/animations/Light2', 'en': ''}, {'ja': 'img/animations/Light3', 'en': ''}, {'ja': 'img/animations/Light4', 'en': ''}, {'ja': 'img/animations/Magic1', 'en': ''}, {'ja': 'img/animations/Magic2', 'en': ''}, {'ja': 'img/animations/Meteor', 'en': ''}, {'ja': 'img/animations/Mist', 'en': ''}, {'ja': 'img/animations/Pollen', 'en': ''}, {'ja': 'img/animations/PreSpecial1', 'en': ''}, {'ja': 'img/animations/PreSpecial2', 'en': ''}, {'ja': 'img/animations/PreSpecial3', 'en': ''}, {'ja': 'img/animations/Recovery1', 'en': ''}, {'ja': 'img/animations/Recovery2', 'en': ''}, {'ja': 'img/animations/Recovery3', 'en': ''}, {'ja': 'img/animations/Recovery4', 'en': ''}, {'ja': 'img/animations/Recovery5', 'en': ''}, {'ja': 'img/animations/Revival1', 'en': ''}, {'ja': 'img/animations/Revival2', 'en': ''}, {'ja': 'img/animations/Slash', 'en': ''}, {'ja': 'img/animations/SlashFire', 'en': ''}, {'ja': 'img/animations/SlashIce', 'en': ''}, {'ja': 'img/animations/SlashPhoton', 'en': ''}, {'ja': 'img/animations/SlashSpecial1', 'en': ''}, {'ja': 'img/animations/SlashSpecial2', 'en': ''}, {'ja': 'img/animations/SlashSpecial3', 'en': ''}, {'ja': 'img/animations/SlashThunder', 'en': ''}, {'ja': 'img/animations/Song', 'en': ''}, {'ja': 'img/animations/Sonic', 'en': ''}, {'ja': 'img/animations/Special1', 'en': ''}, {'ja': 'img/animations/Special2', 'en': ''}, {'ja': 'img/animations/Special3', 'en': ''}, {'ja': 'img/animations/StateChaos', 'en': ''}, {'ja': 'img/animations/StateDark', 'en': ''}, {'ja': 'img/animations/StateDeath', 'en': ''}, {'ja': 'img/animations/StateDown1', 'en': ''}, {'ja': 'img/animations/StateDown2', 'en': ''}, {'ja': 'img/animations/StateDown3', 'en': ''}, {'ja': 'img/animations/StateParalys', 'en': ''}, {'ja': 'img/animations/StatePoison', 'en': ''}, {'ja': 'img/animations/StateSilent', 'en': ''}, {'ja': 'img/animations/StateSleep', 'en': ''}, {'ja': 'img/animations/StateUp1', 'en': ''}, {'ja': 'img/animations/StateUp2', 'en': ''}, {'ja': 'img/animations/Stick', 'en': ''}, {'ja': 'img/animations/StickPhoton', 'en': ''}, {'ja': 'img/animations/StickSpecial1', 'en': ''}, {'ja': 'img/animations/StickSpecial2', 'en': ''}, {'ja': 'img/animations/StickSpecial3', 'en': ''}, {'ja': 'img/animations/Thunder1', 'en': ''}, {'ja': 'img/animations/Thunder2', 'en': ''}, {'ja': 'img/animations/Thunder3', 'en': ''}, {'ja': 'img/animations/Thunder4', 'en': ''}, {'ja': 'img/animations/Thunder5', 'en': ''}, {'ja': 'img/animations/Water1', 'en': ''}, {'ja': 'img/animations/Water2', 'en': ''}, {'ja': 'img/animations/Water3', 'en': ''}, {'ja': 'img/animations/Water4', 'en': ''}, {'ja': 'img/animations/Water5', 'en': ''}, {'ja': 'img/animations/Wind1', 'en': ''}, {'ja': 'img/animations/Wind2', 'en': ''}, {'ja': 'img/animations/Wind3', 'en': ''}, {'ja': 'img/animations/Wind4', 'en': ''}, {'ja': 'img/animations/Wind5', 'en': ''}, {'ja': 'img/battlebacks1/Castle1', 'en': ''}, {'ja': 'img/battlebacks1/Castle2', 'en': ''}, {'ja': 'img/battlebacks1/Clouds', 'en': ''}, {'ja': 'img/battlebacks1/Cobblestones1', 'en': ''}, {'ja': 'img/battlebacks1/Cobblestones2', 'en': ''}, {'ja': 'img/battlebacks1/Cobblestones3', 'en': ''}, {'ja': 'img/battlebacks1/Cobblestones4', 'en': ''}, {'ja': 'img/battlebacks1/Cobblestones5', 'en': ''}, {'ja': 'img/battlebacks1/CobblestonesPool', 'en': ''}, {'ja': 'img/battlebacks1/Crystal', 'en': ''}, {'ja': 'img/battlebacks1/DarkSpace', 'en': ''}, {'ja': 'img/battlebacks1/DecorativeTile', 'en': ''}, {'ja': 'img/battlebacks1/DemonCastle1', 'en': ''}, {'ja': 'img/battlebacks1/DemonCastle2', 'en': ''}, {'ja': 'img/battlebacks1/DemonicWorld', 'en': ''}, {'ja': 'img/battlebacks1/Desert', 'en': ''}, {'ja': 'img/battlebacks1/Dirt1', 'en': ''}, {'ja': 'img/battlebacks1/Dirt2', 'en': ''}, {'ja': 'img/battlebacks1/DirtField', 'en': ''}, {'ja': 'img/battlebacks1/FaceTile', 'en': ''}, {'ja': 'img/battlebacks1/Factory', 'en': ''}, {'ja': 'img/battlebacks1/Grassland', 'en': ''}, {'ja': 'img/battlebacks1/GrassMaze', 'en': ''}, {'ja': 'img/battlebacks1/GrassMazePool', 'en': ''}, {'ja': 'img/battlebacks1/IceCave', 'en': ''}, {'ja': 'img/battlebacks1/InBody', 'en': ''}, {'ja': 'img/battlebacks1/Lava1', 'en': ''}, {'ja': 'img/battlebacks1/Lava2', 'en': ''}, {'ja': 'img/battlebacks1/LavaCave', 'en': ''}, {'ja': 'img/battlebacks1/Meadow', 'en': ''}, {'ja': 'img/battlebacks1/PoisonSwamp', 'en': ''}, {'ja': 'img/battlebacks1/Road1', 'en': ''}, {'ja': 'img/battlebacks1/Road2', 'en': ''}, {'ja': 'img/battlebacks1/Road3', 'en': ''}, {'ja': 'img/battlebacks1/RockCave', 'en': ''}, {'ja': 'img/battlebacks1/Ruins1', 'en': ''}, {'ja': 'img/battlebacks1/Ruins2', 'en': ''}, {'ja': 'img/battlebacks1/Ruins3', 'en': ''}, {'ja': 'img/battlebacks1/Ruins4', 'en': ''}, {'ja': 'img/battlebacks1/Ruins5', 'en': ''}, {'ja': 'img/battlebacks1/Sand', 'en': ''}, {'ja': 'img/battlebacks1/Ship', 'en': ''}, {'ja': 'img/battlebacks1/Sky', 'en': ''}, {'ja': 'img/battlebacks1/Snowfield', 'en': ''}, {'ja': 'img/battlebacks1/Tent', 'en': ''}, {'ja': 'img/battlebacks1/Translucent', 'en': ''}, {'ja': 'img/battlebacks1/Wasteland', 'en': ''}, {'ja': 'img/battlebacks1/WireMesh', 'en': ''}, {'ja': 'img/battlebacks1/Wood1', 'en': ''}, {'ja': 'img/battlebacks1/Wood2', 'en': ''}, {'ja': 'img/battlebacks2/Brick', 'en': ''}, {'ja': 'img/battlebacks2/Bridge', 'en': ''}, {'ja': 'img/battlebacks2/Castle1', 'en': ''}, {'ja': 'img/battlebacks2/Castle2', 'en': ''}, {'ja': 'img/battlebacks2/Castle3', 'en': ''}, {'ja': 'img/battlebacks2/Cliff', 'en': ''}, {'ja': 'img/battlebacks2/Clouds', 'en': ''}, {'ja': 'img/battlebacks2/Crystal', 'en': ''}, {'ja': 'img/battlebacks2/DarkSpace', 'en': ''}, {'ja': 'img/battlebacks2/DemonCastle1', 'en': ''}, {'ja': 'img/battlebacks2/DemonCastle2', 'en': ''}, {'ja': 'img/battlebacks2/DemonCastle3', 'en': ''}, {'ja': 'img/battlebacks2/DemonicWorld', 'en': ''}, {'ja': 'img/battlebacks2/Desert', 'en': ''}, {'ja': 'img/battlebacks2/DirtCave', 'en': ''}, {'ja': 'img/battlebacks2/Forest', 'en': ''}, {'ja': 'img/battlebacks2/Fort1', 'en': ''}, {'ja': 'img/battlebacks2/Fort2', 'en': ''}, {'ja': 'img/battlebacks2/Grassland', 'en': ''}, {'ja': 'img/battlebacks2/GrassMaze', 'en': ''}, {'ja': 'img/battlebacks2/IceCave', 'en': ''}, {'ja': 'img/battlebacks2/IceMaze', 'en': ''}, {'ja': 'img/battlebacks2/InBody', 'en': ''}, {'ja': 'img/battlebacks2/Lava', 'en': ''}, {'ja': 'img/battlebacks2/LavaCave', 'en': ''}, {'ja': 'img/battlebacks2/Metal', 'en': ''}, {'ja': 'img/battlebacks2/Mine', 'en': ''}, {'ja': 'img/battlebacks2/PoisonSwamp', 'en': ''}, {'ja': 'img/battlebacks2/Port', 'en': ''}, {'ja': 'img/battlebacks2/RockCave', 'en': ''}, {'ja': 'img/battlebacks2/Room1', 'en': ''}, {'ja': 'img/battlebacks2/Room2', 'en': ''}, {'ja': 'img/battlebacks2/Room3', 'en': ''}, {'ja': 'img/battlebacks2/Ruins1', 'en': ''}, {'ja': 'img/battlebacks2/Ruins2', 'en': ''}, {'ja': 'img/battlebacks2/Ship', 'en': ''}, {'ja': 'img/battlebacks2/Sky', 'en': ''}, {'ja': 'img/battlebacks2/Snowfield', 'en': ''}, {'ja': 'img/battlebacks2/Stone1', 'en': ''}, {'ja': 'img/battlebacks2/Stone2', 'en': ''}, {'ja': 'img/battlebacks2/Stone3', 'en': ''}, {'ja': 'img/battlebacks2/Temple', 'en': ''}, {'ja': 'img/battlebacks2/Tent', 'en': ''}, {'ja': 'img/battlebacks2/Tower', 'en': ''}, {'ja': 'img/battlebacks2/Town1', 'en': ''}, {'ja': 'img/battlebacks2/Town2', 'en': ''}, {'ja': 'img/battlebacks2/Town3', 'en': ''}, {'ja': 'img/battlebacks2/Town4', 'en': ''}, {'ja': 'img/battlebacks2/Town5', 'en': ''}, {'ja': 'img/battlebacks2/Wasteland', 'en': ''}, {'ja': 'img/characters/!$Gate1', 'en': ''}, {'ja': 'img/characters/!$Gate2', 'en': ''}, {'ja': 'img/characters/!Chest', 'en': ''}, {'ja': 'img/characters/!Crystal', 'en': ''}, {'ja': 'img/characters/!Door1', 'en': ''}, {'ja': 'img/characters/!Door2', 'en': ''}, {'ja': 'img/characters/!Flame', 'en': ''}, {'ja': 'img/characters/!Other1', 'en': ''}, {'ja': 'img/characters/!Other2', 'en': ''}, {'ja': 'img/characters/!SF_Door1', 'en': ''}, {'ja': 'img/characters/!SF_Door2', 'en': ''}, {'ja': 'img/characters/!Switch1', 'en': ''}, {'ja': 'img/characters/!Switch2', 'en': ''}, {'ja': 'img/characters/$BigMonster1', 'en': ''}, {'ja': 'img/characters/$BigMonster2', 'en': ''}, {'ja': 'img/characters/Actor1', 'en': ''}, {'ja': 'img/characters/Actor2', 'en': ''}, {'ja': 'img/characters/Actor3', 'en': ''}, {'ja': 'img/characters/Damage1', 'en': ''}, {'ja': 'img/characters/Damage2', 'en': ''}, {'ja': 'img/characters/Damage3', 'en': ''}, {'ja': 'img/characters/Evil', 'en': ''}, {'ja': 'img/characters/Monster', 'en': ''}, {'ja': 'img/characters/Nature', 'en': ''}, {'ja': 'img/characters/People1', 'en': ''}, {'ja': 'img/characters/People2', 'en': ''}, {'ja': 'img/characters/People3', 'en': ''}, {'ja': 'img/characters/People4', 'en': ''}, {'ja': 'img/characters/Vehicle', 'en': ''}, {'ja': 'img/enemies/Actor1_3', 'en': ''}, {'ja': 'img/enemies/Actor1_4', 'en': ''}, {'ja': 'img/enemies/Actor1_5', 'en': ''}, {'ja': 'img/enemies/Actor1_6', 'en': ''}, {'ja': 'img/enemies/Actor1_7', 'en': ''}, {'ja': 'img/enemies/Actor2_1', 'en': ''}, {'ja': 'img/enemies/Actor2_2', 'en': ''}, {'ja': 'img/enemies/Actor2_3', 'en': ''}, {'ja': 'img/enemies/Actor2_4', 'en': ''}, {'ja': 'img/enemies/Actor2_5', 'en': ''}, {'ja': 'img/enemies/Actor2_6', 'en': ''}, {'ja': 'img/enemies/Actor3_1', 'en': ''}, {'ja': 'img/enemies/Actor3_2', 'en': ''}, {'ja': 'img/enemies/Actor3_5', 'en': ''}, {'ja': 'img/enemies/Actor3_6', 'en': ''}, {'ja': 'img/enemies/Angel', 'en': ''}, {'ja': 'img/enemies/Assassin', 'en': ''}, {'ja': 'img/enemies/Bat', 'en': ''}, {'ja': 'img/enemies/Behemoth', 'en': ''}, {'ja': 'img/enemies/Captain', 'en': ''}, {'ja': 'img/enemies/Cerberus', 'en': ''}, {'ja': 'img/enemies/Chimera', 'en': ''}, {'ja': 'img/enemies/Cockatrice', 'en': ''}, {'ja': 'img/enemies/Darklord-final', 'en': ''}, {'ja': 'img/enemies/Darklord', 'en': ''}, {'ja': 'img/enemies/Death', 'en': ''}, {'ja': 'img/enemies/Demon', 'en': ''}, {'ja': 'img/enemies/Dragon', 'en': ''}, {'ja': 'img/enemies/Earthspirit', 'en': ''}, {'ja': 'img/enemies/Evilgod', 'en': ''}, {'ja': 'img/enemies/Fairy', 'en': ''}, {'ja': 'img/enemies/Fanatic', 'en': ''}, {'ja': 'img/enemies/Firespirit', 'en': ''}, {'ja': 'img/enemies/Gargoyle', 'en': ''}, {'ja': 'img/enemies/Garuda', 'en': ''}, {'ja': 'img/enemies/Gazer', 'en': ''}, {'ja': 'img/enemies/General_f', 'en': ''}, {'ja': 'img/enemies/General_m', 'en': ''}, {'ja': 'img/enemies/Ghost', 'en': ''}, {'ja': 'img/enemies/God', 'en': ''}, {'ja': 'img/enemies/Goddess', 'en': ''}, {'ja': 'img/enemies/Hornet', 'en': ''}, {'ja': 'img/enemies/Imp', 'en': ''}, {'ja': 'img/enemies/Irongiant', 'en': ''}, {'ja': 'img/enemies/Jellyfish', 'en': ''}, {'ja': 'img/enemies/Lamia', 'en': ''}, {'ja': 'img/enemies/Mage', 'en': ''}, {'ja': 'img/enemies/Mimic', 'en': ''}, {'ja': 'img/enemies/Minotaur', 'en': ''}, {'ja': 'img/enemies/Ogre', 'en': ''}, {'ja': 'img/enemies/Orc', 'en': ''}, {'ja': 'img/enemies/Plant', 'en': ''}, {'ja': 'img/enemies/Puppet', 'en': ''}, {'ja': 'img/enemies/Rat', 'en': ''}, {'ja': 'img/enemies/Rogue', 'en': ''}, {'ja': 'img/enemies/Sahuagin', 'en': ''}, {'ja': 'img/enemies/Scorpion', 'en': ''}, {'ja': 'img/enemies/Skeleton', 'en': ''}, {'ja': 'img/enemies/Slime', 'en': ''}, {'ja': 'img/enemies/Snake', 'en': ''}, {'ja': 'img/enemies/Soldier', 'en': ''}, {'ja': 'img/enemies/Spider', 'en': ''}, {'ja': 'img/enemies/Succubus', 'en': ''}, {'ja': 'img/enemies/Swordsman', 'en': ''}, {'ja': 'img/enemies/Vampire', 'en': ''}, {'ja': 'img/enemies/Waterspirit', 'en': ''}, {'ja': 'img/enemies/Werewolf', 'en': ''}, {'ja': 'img/enemies/Willowisp', 'en': ''}, {'ja': 'img/enemies/Windspirit', 'en': ''}, {'ja': 'img/enemies/Zombie', 'en': ''}, {'ja': 'img/faces/Actor1', 'en': ''}, {'ja': 'img/faces/Actor2', 'en': ''}, {'ja': 'img/faces/Actor3', 'en': ''}, {'ja': 'img/faces/Evil', 'en': ''}, {'ja': 'img/faces/Monster', 'en': ''}, {'ja': 'img/faces/Nature', 'en': ''}, {'ja': 'img/faces/People1', 'en': ''}, {'ja': 'img/faces/People2', 'en': ''}, {'ja': 'img/faces/People3', 'en': ''}, {'ja': 'img/faces/People4', 'en': ''}, {'ja': 'img/parallaxes/BlueSky', 'en': ''}, {'ja': 'img/parallaxes/CloudySky1', 'en': ''}, {'ja': 'img/parallaxes/CloudySky2', 'en': ''}, {'ja': 'img/parallaxes/DarkSpace1', 'en': ''}, {'ja': 'img/parallaxes/DarkSpace2', 'en': ''}, {'ja': 'img/parallaxes/Mountains1', 'en': ''}, {'ja': 'img/parallaxes/Mountains2', 'en': ''}, {'ja': 'img/parallaxes/Mountains3', 'en': ''}, {'ja': 'img/parallaxes/Mountains4', 'en': ''}, {'ja': 'img/parallaxes/Mountains5', 'en': ''}, {'ja': 'img/parallaxes/Ocean1', 'en': ''}, {'ja': 'img/parallaxes/Ocean2', 'en': ''}, {'ja': 'img/parallaxes/SeaofClouds', 'en': ''}, {'ja': 'img/parallaxes/StarlitSky', 'en': ''}, {'ja': 'img/parallaxes/Sunset', 'en': ''}, {'ja': 'img/pictures/nantoka10', 'en': 'img/pictures/nantoka8'}, {'ja': 'img/sv_actors/Actor1_1', 'en': ''}, {'ja': 'img/sv_actors/Actor1_2', 'en': ''}, {'ja': 'img/sv_actors/Actor1_3', 'en': ''}, {'ja': 'img/sv_actors/Actor1_4', 'en': ''}, {'ja': 'img/sv_actors/Actor1_5', 'en': ''}, {'ja': 'img/sv_actors/Actor1_6', 'en': ''}, {'ja': 'img/sv_actors/Actor1_7', 'en': ''}, {'ja': 'img/sv_actors/Actor1_8', 'en': ''}, {'ja': 'img/sv_actors/Actor2_1', 'en': ''}, {'ja': 'img/sv_actors/Actor2_2', 'en': ''}, {'ja': 'img/sv_actors/Actor2_3', 'en': ''}, {'ja': 'img/sv_actors/Actor2_4', 'en': ''}, {'ja': 'img/sv_actors/Actor2_5', 'en': ''}, {'ja': 'img/sv_actors/Actor2_6', 'en': ''}, {'ja': 'img/sv_actors/Actor2_7', 'en': ''}, {'ja': 'img/sv_actors/Actor2_8', 'en': ''}, {'ja': 'img/sv_actors/Actor3_5', 'en': ''}, {'ja': 'img/sv_actors/Actor3_6', 'en': ''}, {'ja': 'img/sv_actors/Actor3_7', 'en': ''}, {'ja': 'img/sv_actors/Actor3_8', 'en': ''}, {'ja': 'img/sv_enemies/Actor1_3', 'en': ''}, {'ja': 'img/sv_enemies/Actor1_4', 'en': ''}, {'ja': 'img/sv_enemies/Actor1_5', 'en': ''}, {'ja': 'img/sv_enemies/Actor1_6', 'en': ''}, {'ja': 'img/sv_enemies/Actor1_7', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_1', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_2', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_3', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_4', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_5', 'en': ''}, {'ja': 'img/sv_enemies/Actor2_6', 'en': ''}, {'ja': 'img/sv_enemies/Actor3_1', 'en': ''}, {'ja': 'img/sv_enemies/Actor3_2', 'en': ''}, {'ja': 'img/sv_enemies/Actor3_5', 'en': ''}, {'ja': 'img/sv_enemies/Actor3_6', 'en': ''}, {'ja': 'img/sv_enemies/Angel', 'en': ''}, {'ja': 'img/sv_enemies/Assassin', 'en': ''}, {'ja': 'img/sv_enemies/Bat', 'en': ''}, {'ja': 'img/sv_enemies/Behemoth', 'en': ''}, {'ja': 'img/sv_enemies/Captain', 'en': ''}, {'ja': 'img/sv_enemies/Cerberus', 'en': ''}, {'ja': 'img/sv_enemies/Chimera', 'en': ''}, {'ja': 'img/sv_enemies/Cockatrice', 'en': ''}, {'ja': 'img/sv_enemies/Darklord-final', 'en': ''}, {'ja': 'img/sv_enemies/Darklord', 'en': ''}, {'ja': 'img/sv_enemies/Death', 'en': ''}, {'ja': 'img/sv_enemies/Demon', 'en': ''}, {'ja': 'img/sv_enemies/Dragon', 'en': ''}, {'ja': 'img/sv_enemies/Earthspirit', 'en': ''}, {'ja': 'img/sv_enemies/Evilgod', 'en': ''}, {'ja': 'img/sv_enemies/Fairy', 'en': ''}, {'ja': 'img/sv_enemies/Fanatic', 'en': ''}, {'ja': 'img/sv_enemies/Firespirit', 'en': ''}, {'ja': 'img/sv_enemies/Gargoyle', 'en': ''}, {'ja': 'img/sv_enemies/Garuda', 'en': ''}, {'ja': 'img/sv_enemies/Gazer', 'en': ''}, {'ja': 'img/sv_enemies/General_f', 'en': ''}, {'ja': 'img/sv_enemies/General_m', 'en': ''}, {'ja': 'img/sv_enemies/Ghost', 'en': ''}, {'ja': 'img/sv_enemies/God', 'en': ''}, {'ja': 'img/sv_enemies/Goddess', 'en': ''}, {'ja': 'img/sv_enemies/Hornet', 'en': ''}, {'ja': 'img/sv_enemies/Imp', 'en': ''}, {'ja': 'img/sv_enemies/Irongiant', 'en': ''}, {'ja': 'img/sv_enemies/Jellyfish', 'en': ''}, {'ja': 'img/sv_enemies/Lamia', 'en': ''}, {'ja': 'img/sv_enemies/Mage', 'en': ''}, {'ja': 'img/sv_enemies/Mimic', 'en': ''}, {'ja': 'img/sv_enemies/Minotaur', 'en': ''}, {'ja': 'img/sv_enemies/Ogre', 'en': ''}, {'ja': 'img/sv_enemies/Orc', 'en': ''}, {'ja': 'img/sv_enemies/Plant', 'en': ''}, {'ja': 'img/sv_enemies/Puppet', 'en': ''}, {'ja': 'img/sv_enemies/Rat', 'en': ''}, {'ja': 'img/sv_enemies/Rogue', 'en': ''}, {'ja': 'img/sv_enemies/Sahuagin', 'en': ''}, {'ja': 'img/sv_enemies/Scorpion', 'en': ''}, {'ja': 'img/sv_enemies/Skeleton', 'en': ''}, {'ja': 'img/sv_enemies/Slime', 'en': ''}, {'ja': 'img/sv_enemies/Snake', 'en': ''}, {'ja': 'img/sv_enemies/Soldier', 'en': ''}, {'ja': 'img/sv_enemies/Spider', 'en': ''}, {'ja': 'img/sv_enemies/Succubus', 'en': ''}, {'ja': 'img/sv_enemies/Swordsman', 'en': ''}, {'ja': 'img/sv_enemies/Vampire', 'en': ''}, {'ja': 'img/sv_enemies/Waterspirit', 'en': ''}, {'ja': 'img/sv_enemies/Werewolf', 'en': ''}, {'ja': 'img/sv_enemies/Willowisp', 'en': ''}, {'ja': 'img/sv_enemies/Windspirit', 'en': ''}, {'ja': 'img/sv_enemies/Zombie', 'en': ''}, {'ja': 'img/system/Balloon', 'en': ''}, {'ja': 'img/system/ButtonSet', 'en': ''}, {'ja': 'img/system/Damage', 'en': ''}, {'ja': 'img/system/GameOver', 'en': ''}, {'ja': 'img/system/IconSet', 'en': ''}, {'ja': 'img/system/Loading', 'en': ''}, {'ja': 'img/system/MadeWithMv', 'en': ''}, {'ja': 'img/system/Shadow1', 'en': ''}, {'ja': 'img/system/Shadow2', 'en': ''}, {'ja': 'img/system/States', 'en': ''}, {'ja': 'img/system/Weapons1', 'en': ''}, {'ja': 'img/system/Weapons2', 'en': ''}, {'ja': 'img/system/Weapons3', 'en': ''}, {'ja': 'img/system/Window', 'en': ''}, {'ja': 'img/tilesets/Dungeon_A1', 'en': ''}, {'ja': 'img/tilesets/Dungeon_A2', 'en': ''}, {'ja': 'img/tilesets/Dungeon_A4', 'en': ''}, {'ja': 'img/tilesets/Dungeon_A5', 'en': ''}, {'ja': 'img/tilesets/Dungeon_B', 'en': ''}, {'ja': 'img/tilesets/Dungeon_C', 'en': ''}, {'ja': 'img/tilesets/Inside_A1', 'en': ''}, {'ja': 'img/tilesets/Inside_A2', 'en': ''}, {'ja': 'img/tilesets/Inside_A4', 'en': ''}, {'ja': 'img/tilesets/Inside_A5', 'en': ''}, {'ja': 'img/tilesets/Inside_B', 'en': ''}, {'ja': 'img/tilesets/Inside_C', 'en': ''}, {'ja': 'img/tilesets/Outside_A1', 'en': ''}, {'ja': 'img/tilesets/Outside_A2', 'en': ''}, {'ja': 'img/tilesets/Outside_A3', 'en': ''}, {'ja': 'img/tilesets/Outside_A4', 'en': ''}, {'ja': 'img/tilesets/Outside_A5', 'en': ''}, {'ja': 'img/tilesets/Outside_B', 'en': ''}, {'ja': 'img/tilesets/Outside_C', 'en': ''}, {'ja': 'img/tilesets/SF_Inside_A4', 'en': ''}, {'ja': 'img/tilesets/SF_Inside_B', 'en': ''}, {'ja': 'img/tilesets/SF_Inside_C', 'en': ''}, {'ja': 'img/tilesets/SF_Outside_A3', 'en': ''}, {'ja': 'img/tilesets/SF_Outside_A4', 'en': ''}, {'ja': 'img/tilesets/SF_Outside_A5', 'en': ''}, {'ja': 'img/tilesets/SF_Outside_B', 'en': ''}, {'ja': 'img/tilesets/SF_Outside_C', 'en': ''}, {'ja': 'img/tilesets/World_A1', 'en': ''}, {'ja': 'img/tilesets/World_A2', 'en': ''}, {'ja': 'img/tilesets/World_B', 'en': ''}, {'ja': 'img/tilesets/World_C', 'en': ''}, {'ja': 'img/titles1/Book', 'en': ''}, {'ja': 'img/titles1/Castle', 'en': ''}, {'ja': 'img/titles1/CrossedSwords', 'en': ''}, {'ja': 'img/titles1/Crystal', 'en': ''}, {'ja': 'img/titles1/DemonCastle', 'en': ''}, {'ja': 'img/titles1/Devil', 'en': ''}, {'ja': 'img/titles1/Dragon', 'en': ''}, {'ja': 'img/titles1/Fountain', 'en': ''}, {'ja': 'img/titles1/Gates', 'en': ''}, {'ja': 'img/titles1/Hexagram', 'en': ''}, {'ja': 'img/titles1/Island', 'en': ''}, {'ja': 'img/titles1/Night', 'en': ''}, {'ja': 'img/titles1/Plain', 'en': ''}, {'ja': 'img/titles1/Sword', 'en': ''}, {'ja': 'img/titles1/Tower1', 'en': ''}, {'ja': 'img/titles1/Tower2', 'en': ''}, {'ja': 'img/titles1/Universe', 'en': ''}, {'ja': 'img/titles1/Volcano', 'en': ''}, {'ja': 'img/titles1/World', 'en': ''}, {'ja': 'img/titles1/WorldMap', 'en': ''}, {'ja': 'img/titles2/Floral', 'en': ''}, {'ja': 'img/titles2/Medieval', 'en': ''}];
const testItems = [{'ja': 'ポーション', 'en': 'portion'}, {'ja': 'マジックウォーター', 'en': 'magic water'}, {'ja': 'ディスペルハーブ', 'en': 'dispersal herb'}, {'ja': 'スティミュラント', 'en': 'formula'}];
const testMap001 = [{'ja': '通常のテキストです', 'en': 'Normal text.'}, {'ja': '改行を含む\nテキストです', 'en': 'Includes line breaks\nText.'}, {'ja': 'カンマを含む,テキストです', 'en': 'Including comma, text.'}, {'ja': '"タ"フ"ルクォーテーションを含むテキストです"', 'en': '"The text contains a t"ouh"le quotation."'}, {'ja': '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"', 'en': '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.'}, {'ja': '名前変えるよ', 'en': "I'll change the name."}, {'ja': '1番の変数の値は \\V[1] です。', 'en': 'The value of variable 1 is \\V[1].'}, {'ja': '1番のアクターの名前は \\N[1] です。', 'en': 'The name of the 1 actor is \\N[1].'}, {'ja': '1番のパーティーメンバーの名前は \\P[1] です。', 'en': 'The name of party member 1 is \\P[1].'}, {'ja': '現在の所持金は \\G です。', 'en': 'The current amount of money you have is \\G'}, {'ja': 'この文字は \\C[2] 赤色 \\C[0] 通常色 です。', 'en': 'This letter is \\C[2] red \\C[0] normal color.'}, {'ja': 'これはアイコン \\I[64] の表示です。', 'en': 'This is the display of the icon \\I[64].'}, {'ja': '文字サイズを \\{ 大きく \\} 小さくします。', 'en': 'The font size will be reduced to \\{large \\}smaller.'}, {'ja': '\\$ 所持金ウィンドウを表示します。', 'en': '\\$ Display the money window.'}, {'ja': '文章の途中で 1/4 秒間のウェイトを \\. します。', 'en': 'It will display a 1/4 second wait \\. in the middle of a sentence.'}, {'ja': '文章の途中で 1 秒間のウェイトを \\| します。', 'en': 'The text is weighted for one second \\| in the middle of a sentence.'}, {'ja': '文章の途中でボタンの入力待ちを \\! します。', 'en': 'Waiting for button input in the middle of a sentence \\! The button is pressed.'}, {'ja': '\\>この行の文字を一瞬で表示します。\\<', 'en': '\\>Display the characters of this line in a moment. \\<'}, {'ja': '\\^文章表示後の入力待ちを行いません。', 'en': '\\^Do not wait for input after displaying a sentence.'}, {'ja': 'バックスラッシュの表示は \\\\ です。', 'en': 'The backslash is displayed at \\\\'}, {'ja': '複合させます\n\\{\\C[2]\\N[2]\\I[22]', 'en': 'Compounding\n\\{\\C[2]\\N[2]\\I[22]'}, {'ja': 'ラフィーナ', 'en': 'Rafina.'}, {'ja': 'エルーシェ', 'en': 'eluche'}, {'ja': 'やめる', 'en': 'leave'}, {'ja': '二つ名変えるよ', 'en': "I'll change my name."}, {'ja': '雑用係', 'en': 'Compassionate'}, {'ja': '傲慢ちき', 'en': 'arrogant'}, {'ja': '無くす', 'en': 'lose something'}, {'ja': 'プロフィール変えるよ', 'en': "I'll change my profile."}, {'ja': 'チビのツンデレウーマン。\n魔法が得意。', 'en': 'Tiny tsundere woman.\nShe is good at magic.'}, {'ja': 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en': "Kestinius' scullery maid.\nNot that much work."}, {'ja': '何があったのかよく思い出せない。\n\n辺りを包む木々。天を覆う新緑の葉。\n\n小鳥のせせらぎと、風の音だけが聴こえる。\n\n彼方には、見慣れたような、しかしそうでないような、\n懐かしさを僅かに感じる家並み———\n\nふと、目の前の景色が遠ざかる。\n何かに後ろに引っ張られるように、\n辺りを俯瞰するような景色が広がっていく。\n\n"アタシ"は後ろを振り向いた。\n\n肩まで行かない赤髪を垂らし、\n見開く緑目は"私"を見つめ———', 'en': 'I can\'t quite remember what happened.\n\nTrees surrounding the area. Fresh green leaves covering the heavens.\n\nOnly the rustling of birds and the sound of the wind can be heard.\n\nIn the distance, the houses look familiar, but not so familiar,\nhouses, which seem familiar but not so familiar...\n\nSuddenly, the view in front of you fades away.\nAs if something is pulling you back,\nthe view of the area spreads out like a bird\'s eye view.\n\n"Atashi" turned around.\n\nHer red hair hangs down to her shoulders,\nGreen eyes wide open, staring at "me"...'}, {'ja': '勝ち', 'en': 'win'}, {'ja': '逃げ犬', 'en': 'fugitive dog'}, {'ja': '負け犬', 'en': 'dejected loser'}, {'ja': '\\{言語\\}変える\\}よ', 'en': "\\}I'm going to \\{change \\{Language\\}."}, {'ja': '日本語', 'en': 'Japanese (language)'}, {'ja': '英語', 'en': 'English (language)'}, {'ja': '中国語', 'en': 'Chinese Language'}, {'ja': 'やっぱやめる', 'en': 'give up'}, {'ja': 'フィールドを移動します', 'en': 'Move field'}, {'ja': '行って帰る', 'en': 'go and come back'}, {'ja': '行ったきり', 'en': 'going and leaving'}, {'ja': '移動後のメッセージです。', 'en': 'This is the message after the move.'}, {'ja': '更に移動した際のメッセージです', 'en': "Here's the message when we move further"}];
const testMap002 = [{'ja': '帰れ！', 'en': 'Go Home!'}];
const testMap003 = [{'ja': '誰……？', 'en': 'Who ......?.'}];
const testScripts = [{'ja': 'CustomizeConfigItem', 'en': ''}, {'ja': 'NumberOptions', 'en': ''}, {'ja': '数値項目', 'en': ''}, {'ja': 'StringOptions', 'en': ''}, {'ja': '文字項目', 'en': ''}, {'ja': 'SwitchOptions', 'en': ''}, {'ja': 'スイッチ項目', 'en': ''}, {'ja': 'VolumeOptions', 'en': ''}, {'ja': '音量項目', 'en': ''}, {'ja': 'Number', 'en': ''}, {'ja': 'Boolean', 'en': ''}, {'ja': 'String', 'en': ''}, {'ja': 'Volume', 'en': ''}, {'ja': 'no item', 'en': ''}, {'ja': 'shift', 'en': ''}, {'ja': 'NumberOptions', 'en': ''}, {'ja': '数値項目', 'en': ''}, {'ja': 'StringOptions', 'en': ''}, {'ja': '文字項目', 'en': ''}, {'ja': 'SwitchOptions', 'en': ''}, {'ja': 'スイッチ項目', 'en': ''}, {'ja': 'VolumeOptions', 'en': ''}, {'ja': '音量項目', 'en': ''}, {'ja': '', 'en': 'Number'}, {'ja': '', 'en': 'Boolean'}, {'ja': '', 'en': 'String'}, {'ja': '', 'en': 'Volume'}, {'ja': '', 'en': 'shift'}, {'ja': 'alwaysDash', 'en': ''}, {'ja': 'スイッチ項目1', 'en': 'Switch Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '数値項目1', 'en': 'Numeric Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '数値項目2', 'en': 'Numeric Item 2'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '難易度', 'en': 'difficulty'}, {'ja': '', 'en': ''}, {'ja': '易しい', 'en': 'Easy'}, {'ja': '難しい', 'en': 'Difficult'}, {'ja': '地獄', 'en': 'Hell'}, {'ja': '', 'en': ''}, {'ja': '音量項目1', 'en': 'Volume Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '音量項目2', 'en': 'Volume Item 2'}, {'ja': '', 'en': ''}, {'ja': 'TMLogWindow', 'en': ''}, {'ja': ' / ', 'en': ''}, {'ja': 'ok', 'en': ''}, {'ja': 'cancel', 'en': ''}, {'ja': '', 'en': 'YED_Hospital'}, {'ja': '', 'en': ','}, {'ja': '', 'en': 'Actor Help (Healthy)'}, {'ja': '', 'en': 'Actor Help (Treat)'}, {'ja': '', 'en': 'healOne'}, {'ja': '', 'en': 'healAll'}, {'ja': '', 'en': 'cancel'}, {'ja': '', 'en': 'Heal One Help'}, {'ja': '', 'en': 'Heal All Help (Healthy)'}, {'ja': '', 'en': 'Heal All Help (Treat)'}, {'ja': '', 'en': 'Exit Help'}, {'ja': '', 'en': 'healOne'}, {'ja': '', 'en': 'healAll'}, {'ja': '', 'en': 'cancel'}, {'ja': '', 'en': 'ok'}, {'ja': '', 'en': 'cancel'}, {'ja': '%1 does not need to be recovered', 'en': '%1は回復の必要はない'}, {'ja': 'Recover one.', 'en': '1人を回復する'}, {'ja': '%1needs to be recovered.', 'en': '%1は回復が必要だ'}, {'ja': 'Pay%1G to recover all members', 'en': '%1Gを払ってメンバー全員を回復させます'}, {'ja': 'Exit', 'en': 'Exit'}, {'ja': 'Exit.', 'en': '外に出る'}, {'ja': 'Recover all members.', 'en': '全員を回復する'}, {'ja': 'There is no member who needs to be healed.', 'en': '回復が必要なメンバーが居ません'}, {'ja': 'Heal each member individually', 'en': 'メンバーを個別に回復させます'}, {'ja': 'People4, 1', 'en': 'People4, 1'}, {'ja': 'Hello, how can I help you today?', 'en': 'こんにちは、今日はどうされましたか？'}, {'ja': 'Loli', 'en': 'Loli'}, {'ja': '', 'en': 'center'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': 'en:Advanced Array 1', 'en': 'Advanced Array 1'}, {'ja': 'en:Advanced Array 2', 'en': 'Advanced Array 2'}, {'ja': 'Advanced Array 1', 'en': 'アドバンス Array 1'}, {'ja': 'Advanced Array 2', 'en': 'アドバンス Array 2'}, {'ja': 'Advanced Array 3', 'en': 'アドバンス Array 3'}, {'ja': 'Advanced Array 4', 'en': 'アドバンス Array 4'}, {'ja': '', 'en': 'Map001'}, {'ja': '', 'en': ''}, {'ja': 'Choice 1', 'en': 'チョイス 1'}, {'ja': 'Choice 2', 'en': 'チョイス 2'}, {'ja': '', 'en': '<dynamicNote:custom>'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': 'Dynamic 1', 'en': 'Dynamic 1'}, {'ja': 'Inner Value 1', 'en': 'Inner Value 1'}, {'ja': 'Inner Value 2', 'en': 'Inner Value 2'}];
const testSkills = [{'ja': 'の攻撃！', 'en': 'attack!'}, {'ja': '攻撃', 'en': 'Attack'}, {'ja': 'は身を守っている。', 'en': 'is protecting itself.'}, {'ja': '防御', 'en': 'Guard'}, {'ja': '連続攻撃', 'en': 'Continuous attack'}, {'ja': '２回攻撃', 'en': '2 attacks'}, {'ja': '3回攻撃', 'en': '3 attacks'}, {'ja': 'は逃げてしまった。', 'en': 'has fled.'}, {'ja': '逃げる', 'en': 'escape'}, {'ja': 'は様子を見ている。', 'en': 'is looking at the situation.'}, {'ja': '様子を見る', 'en': 'wait and see'}, {'ja': 'は%1を唱えた！', 'en': 'chanted %1!'}, {'ja': 'ヒール', 'en': 'heel'}, {'ja': 'ファイア', 'en': 'fire'}, {'ja': 'スパーク', 'en': 'spark'}, {'ja': 'お手軽デバフセット！', 'en': ''}];
const testStates = [{'ja': 'は倒れた！', 'en': 'fell down!'}, {'ja': 'を倒した！', 'en': 'defeated!'}, {'ja': 'は立ち上がった！', 'en': 'stood up!'}, {'ja': '戦闘不能', 'en': 'inability to fight'}, {'ja': '予防的', 'en': 'precautionary'}, {'ja': '不死身', 'en': 'immortal body'}, {'ja': 'は毒にかかった！', 'en': 'was poisoned!'}, {'ja': 'に毒をかけた！', 'en': 'poisoned the'}, {'ja': 'の毒が消えた！', 'en': 'The poison of the "Mere Old Man" disappeared!'}, {'ja': '毒', 'en': 'Poison'}, {'ja': 'は暗闇に閉ざされた！', 'en': 'was closed in darkness!'}, {'ja': 'を暗闇に閉ざした！', 'en': 'closed to the darkness!'}, {'ja': 'の暗闇が消えた！', 'en': 'of darkness disappeared!'}, {'ja': '暗闇', 'en': 'the dark'}, {'ja': 'は沈黙した！', 'en': 'was silent!'}, {'ja': 'を沈黙させた！', 'en': 'was silenced!'}, {'ja': 'の沈黙が解けた！', 'en': 'The silence of the "Mere Old Man" was broken!'}, {'ja': '鳴く', 'en': 'purr'}, {'ja': 'は激昂した！', 'en': 'was agitated!'}, {'ja': 'を激昂させた！', 'en': 'The first time the company was involved in the project, it was aggravated by the'}, {'ja': 'は我に返った！', 'en': 'came back to himself!'}, {'ja': '憤慨', 'en': 'aroused indignation'}, {'ja': 'は混乱した！', 'en': 'was confused!'}, {'ja': 'を混乱させた！', 'en': 'confused!'}, {'ja': '混乱', 'en': 'confusion'}, {'ja': 'は魅了された！', 'en': 'was fascinated!'}, {'ja': 'を魅了した！', 'en': 'The event fascinated the'}, {'ja': '魅了', 'en': 'fascinate'}, {'ja': 'は眠った！', 'en': 'slept!'}, {'ja': 'を眠らせた！', 'en': 'He put the "Mere Old Man" to sleep!'}, {'ja': 'は眠っている。', 'en': 'is sleeping.'}, {'ja': 'は目を覚ました！', 'en': 'woke up!'}, {'ja': '睡眠', 'en': 'sleep'}];
const testSystem = [{'ja': '一般防具', 'en': 'General Defense'}, {'ja': '魔法防具', 'en': 'Magic Defense'}, {'ja': '軽装防具', 'en': 'Lightweight Defense'}, {'ja': '重装防具', 'en': 'Heavy Duty Defense'}, {'ja': '小型盾', 'en': 'shields'}, {'ja': '大型盾', 'en': 'large shield'}, {'ja': 'ゴル', 'en': 'Gol'}, {'ja': '物理', 'en': 'physiotherapy'}, {'ja': '炎', 'en': 'Frame'}, {'ja': '氷', 'en': 'Ice'}, {'ja': '雷', 'en': 'Thunder'}, {'ja': '水', 'en': 'Water'}, {'ja': '土', 'en': 'Mud'}, {'ja': '風', 'en': 'Wind'}, {'ja': '光', 'en': 'Light'}, {'ja': '闇', 'en': 'Dark'}, {'ja': '武器', 'en': 'weaponry'}, {'ja': '盾', 'en': 'Shield'}, {'ja': '頭', 'en': 'Head'}, {'ja': '身体', 'en': 'body'}, {'ja': '装飾品', 'en': 'ornament'}, {'ja': 'Langdcore_Test', 'en': 'Langdcore_Test'}, {'ja': '魔法', 'en': 'magic'}, {'ja': '必殺技', 'en': 'specia'}, {'ja': 'レベル', 'en': 'level'}, {'ja': 'Lv', 'en': 'Lv'}, {'ja': 'ＨＰ', 'en': 'ＨＰ'}, {'ja': 'HP', 'en': 'HP'}, {'ja': 'ＭＰ', 'en': 'ＭＰ'}, {'ja': 'MP', 'en': 'MP'}, {'ja': 'ＴＰ', 'en': 'ＴＰ'}, {'ja': 'TP', 'en': 'TP'}, {'ja': '経験値', 'en': 'XP'}, {'ja': 'EXP', 'en': 'EXP'}, {'ja': '戦う', 'en': 'fight'}, {'ja': '逃げる', 'en': 'escape'}, {'ja': '攻撃', 'en': 'attack'}, {'ja': '防御', 'en': 'precautionary'}, {'ja': 'アイテム', 'en': 'item'}, {'ja': 'スキル', 'en': 'skill'}, {'ja': '装備', 'en': 'equipment'}, {'ja': 'ステータス', 'en': 'status'}, {'ja': '並び替え', 'en': 'sort'}, {'ja': 'セーブ', 'en': 'curbing'}, {'ja': 'ゲーム終了', 'en': 'end of game'}, {'ja': 'オプション', 'en': 'option'}, {'ja': '防具', 'en': 'protective gear'}, {'ja': '大事なもの', 'en': 'Important.'}, {'ja': '最強装備', 'en': 'the most powerful equipment'}, {'ja': '全て外す', 'en': 'remove all of them'}, {'ja': 'ニューゲーム', 'en': 'new game'}, {'ja': 'コンティニュー', 'en': 'continue'}, {'ja': 'タイトルへ', 'en': 'Go to Title.'}, {'ja': 'やめる', 'en': 'leave'}, {'ja': '購入する', 'en': 'Buy Now'}, {'ja': '売却する', 'en': 'Sold to.'}, {'ja': '%1には効かなかった！', 'en': 'It did not work for %1!'}, {'ja': '%1は %2 のダメージを受けた！', 'en': '%1 took %2 damage!'}, {'ja': '%1は%2を %3 奪われた！', 'en': '%1 %2 was %3 robbed!'}, {'ja': '%1の%2が %3 増えた！', 'en': 'The %2 of %1 increased by %3!'}, {'ja': '%1の%2が %3 減った！', 'en': 'The %2 of %1 was reduced by %3!'}, {'ja': '%1はダメージを受けていない！', 'en': '%1 is not damaged!'}, {'ja': 'ミス！\u3000%1はダメージを受けていない！', 'en': 'Miss!\u3000%1 is not damaged!'}, {'ja': '%1の%2が %3 回復した！', 'en': 'The %2 of %1 recovered %3!'}, {'ja': '常時ダッシュ', 'en': 'dash at any time'}, {'ja': 'BGM 音量', 'en': 'BGM Volume'}, {'ja': 'BGS 音量', 'en': 'BGS Volume'}, {'ja': '%1の%2が上がった！', 'en': 'The %2 of %1 was raised!'}, {'ja': '%1の%2が元に戻った！', 'en': 'The %1 %2 is back to normal!'}, {'ja': 'コマンド記憶', 'en': 'command storage'}, {'ja': '%1の反撃！', 'en': 'Counterattack at %1!'}, {'ja': '痛恨の一撃！！', 'en': 'A painful blow!'}, {'ja': '会心の一撃！！', 'en': 'A blow to the heart!'}, {'ja': '%1の%2が下がった！', 'en': 'The %2 of %1 dropped!'}, {'ja': '%1は戦いに敗れた。', 'en': '%1 lost the battle.'}, {'ja': '%1が出現！', 'en': '%1 appears!'}, {'ja': '%1に %2 のダメージを与えた！', 'en': '%2 damage to %1!'}, {'ja': '%1の%2を %3 奪った！', 'en': "I took %3 of %1's %2!"}, {'ja': '%1にダメージを与えられない！', 'en': 'No damage to %1!'}, {'ja': 'ミス！\u3000%1にダメージを与えられない！', 'en': 'Miss!\u3000No damage to %1!'}, {'ja': 'しかし逃げることはできなかった！', 'en': 'But there was no escape!'}, {'ja': '%1は逃げ出した！', 'en': '%1 escaped!'}, {'ja': '%1は攻撃をかわした！', 'en': '%1 dodged the attack!'}, {'ja': '次の%1まで', 'en': 'Next %1 to'}, {'ja': '現在の%1', 'en': 'Current%1'}, {'ja': 'ファイル', 'en': 'file'}, {'ja': '%1は%2 %3 に上がった！', 'en': '%1 went up to %2 %3!'}, {'ja': 'どのファイルをロードしますか？', 'en': 'Which files do you want to load?'}, {'ja': '%1は魔法を打ち消した！', 'en': 'The %1 cancelled the magic!'}, {'ja': '%1は魔法を跳ね返した！', 'en': 'The %1 bounced back the magic!'}, {'ja': 'ME 音量', 'en': 'ME Volume'}, {'ja': '%1 の%2を獲得！', 'en': 'Gained %2 of %1!'}, {'ja': 'お金を %1\\G 手に入れた！', 'en': 'I got %1\\G of money!'}, {'ja': '%1を手に入れた！', 'en': 'I got %1!'}, {'ja': '%1を覚えた！', 'en': 'I learned %1!'}, {'ja': '%1たち', 'en': 'The %1s.'}, {'ja': '持っている数', 'en': 'Number of items you have'}, {'ja': '%1は先手を取った！', 'en': '%1 took the lead!'}, {'ja': 'どのファイルにセーブしますか？', 'en': 'Which file do you want to save to?'}, {'ja': 'SE 音量', 'en': 'SE Volume'}, {'ja': '%1が%2をかばった！', 'en': '%1 covered for %2!'}, {'ja': '%1は不意をつかれた！', 'en': 'The %1 caught me off guard!'}, {'ja': '%1は%2を使った！', 'en': '%1 used %2!'}, {'ja': '%1の勝利！', 'en': '%1 win!'}, {'ja': '最大ＨＰ', 'en': 'Maximum HP'}, {'ja': '最大ＭＰ', 'en': 'Maximum MP'}, {'ja': '攻撃力', 'en': 'offensive ability'}, {'ja': '防御力', 'en': 'defensive strength'}, {'ja': '魔法力', 'en': 'magic power'}, {'ja': '魔法防御', 'en': 'Magic Defense'}, {'ja': '敏捷性', 'en': 'agility'}, {'ja': '運', 'en': 'Luck'}, {'ja': '命中率', 'en': 'hit rate'}, {'ja': '回避率', 'en': 'evasion rate'}, {'ja': '短剣', 'en': 'stiletto'}, {'ja': '剣', 'en': 'Sword'}, {'ja': 'フレイル', 'en': 'frail'}, {'ja': '斧', 'en': 'Axe'}, {'ja': 'ムチ', 'en': 'whip'}, {'ja': '杖', 'en': 'Staff'}, {'ja': '弓', 'en': 'Bow'}, {'ja': 'クロスボウ', 'en': 'crossbow'}, {'ja': '銃', 'en': 'Gun'}, {'ja': '爪', 'en': 'Claw'}, {'ja': 'グローブ', 'en': 'glove'}, {'ja': '槍', 'en': 'Lance'}];
const testTroops = [{'ja': 'おおおおおおいおいおぃおぉいぃぉぃぉぃぉぉぉぃぃ', 'en': 'oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh.'}];
const testWeapons = [{'ja': '剣', 'en': 'Sword'}, {'ja': '斧', 'en': 'Axe'}, {'ja': '杖', 'en': 'Staff'}, {'ja': '弓', 'en': 'Bow'}];
// END OF GENERATED CONTENT

function waitForCondition(conditionFunction, interval = 100, timeout = 5000) {
  return new Promise((resolve, reject) => {
    const startTime = Date.now();

    function checkCondition() {
      if (conditionFunction()) 
      {
        resolve();
      } else if (Date.now() - startTime >= timeout) {
        reject(new Error('Timeout waiting for condition'));
      } else {
        setTimeout(checkCondition, interval);
      }
    }

    checkCondition();
  });
};

//Vorbisのエミュレートに必要
class NodeWorker {
  constructor(scriptURL) {
    this.worker = new Worker(scriptURL, { eval: true });

    this.worker.on('message', (message) => {
      if (this.onmessage) this.onmessage({ data: message });
    });

    this.worker.on('error', (error) => {
      if (this.onerror) this.onerror(error);
    });
  }

  postMessage(message) {
    this.worker.postMessage(message);
  }

  terminate() {
    this.worker.terminate();
  }

  addEventListener(type, listener) {
    if (type === 'message') {
      this.onmessage = listener;
    } else if (type === 'error') {
      this.onerror = listener;
    }
  }

  removeEventListener(type, listener) {
    if (type === 'message' && this.onmessage === listener) {
      this.onmessage = null;
    } else if (type === 'error' && this.onerror === listener) {
      this.onerror = null;
    }
  }
}

let dom, window;
initializeRPGMaker = async function() 
{
  // 環境変数の設定を確認
  // index.htmlのパスを指定
  const htmlFilePath = path.resolve(__dirname, 'index.html');
  // HTMLファイルの内容を読み込む
  const htmlContent = fs.readFileSync(htmlFilePath, 'utf-8');
  dom = new JSDOM(htmlContent, {
    // 必要に応じてURLや他のオプションを指定
    url: 'http://localhost:8180/',
    runScripts: 'dangerously', // JavaScriptの実行を許可
    resources: 'usable', // 外部リソース（スクリプト）の読み込みを許可
    pretendToBeVisual: true, // オプション：仮想ブラウザとして振る舞う      
    beforeParse(window) {
      if(process.env.IS_NWJS)
      {
        var rootPath = path.resolve(process.mainModule.path, "../../../");
        rootPath = path.join(rootPath, process.env.PROJ_NAME);
        window.process = {};
        window.process.env = {
          NODE_ENV: process.env.NODE_ENV,
          IS_NWJS: process.env.IS_NWJS
        };
        window.process.mainModule = {
          filename: rootPath + "/window.Langscore.test.js",
          path: rootPath
        };
        window.require = require;
      }
      else
      {
        window.process = undefined;
        window.require = undefined;
      }

      window.FontFace = global.FontFace;
    }
  });

  window = dom.window;
  dom.window.Worker = NodeWorker;

  // DOMContentLoadedの完了を待つ
  await new Promise(resolve => {
    window.addEventListener('load', () => {
      resolve();
    });
  });

  await waitForCondition(() => window.SceneManager._stopped === true);

};

describe('LSCSV', function() {
  before(() => initializeRPGMaker());

  it('CSVテキストからヘッダーを正しく取得すること', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello';
    const header = window._lscsv.fetch_header(csvText);
    expect(header).to.deep.equal(['origin', 'ja', 'en']);
  });

  it('CSVテキストを正しく解析すること', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye';
    const result = window._lscsv.to_map(csvText);
    expect(result['value1']['ja']).to.deep.equal('こんにちは');
    expect(result['value1']['en']).to.deep.equal('Hello');
    expect(result['value2']['ja']).to.deep.equal('さようなら');
    expect(result['value2']['en']).to.deep.equal('Goodbye');
  });

  it('特殊な形式を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye';
    const result = window._lscsv.to_map(csvText);
    expect(result['value1']['ja']).to.deep.equal('こんにちは');
    expect(result['value1']['en']).to.deep.equal('Hello');
    expect(result['value2']['ja']).to.deep.equal('さようなら');
    expect(result['value2']['en']).to.deep.equal('Goodbye');
  });

  it('ヘッダーを正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n';
    const result = window._lscsv.to_map(csvText);
    expect(result).to.deep.equal(new Map());
  });

  it('最初の行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n通常のテキストです,通常のテキストです,Normal text.';
    const result = window._lscsv.to_map(csvText);
    expect(result['通常のテキストです']['ja']).to.equal('通常のテキストです');
    expect(result['通常のテキストです']['en']).to.equal('Normal text.');
  });

  it('改行を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"改行を含む\nテキストです","改行を含む\nテキストです","Includes line breaks\nText."';
    const result = window._lscsv.to_map(csvText);
    expect(result['改行を含む\nテキストです']['ja']).to.equal('改行を含む\nテキストです');
    expect(result['改行を含む\nテキストです']['en']).to.equal('Includes line breaks\nText.');
  });

  it('カンマを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"カンマを含む,テキストです","カンマを含む,テキストです","Including comma, text."';
    const result = window._lscsv.to_map(csvText);
    expect(result['カンマを含む,テキストです']['ja']).to.equal('カンマを含む,テキストです');
    expect(result['カンマを含む,テキストです']['en']).to.equal('Including comma, text.');
  });

  it('ダブルクォーテーションを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"""タ""フ""ルクォーテーションを含むテキストです""","""タ""フ""ルクォーテーションを含むテキストです""","""The text contains a t""ouh""le quotation."""';
    const result = window._lscsv.to_map(csvText);
    var key = '"タ"フ"ルクォーテーションを含むテキストです"';
    var expect_ja = '"タ"フ"ルクォーテーションを含むテキストです"'
    var expect_en = '"The text contains a t"ouh"le quotation."'
    var lang_hash = result[key];
    expect(lang_hash['ja']).to.equal(expect_ja);
    expect(lang_hash['en']).to.equal(expect_en);
  });

  it('"括りや改行が混在する文章を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",""\nIt is a "",""""Mystery spell""""(Mystery spell)---"""""" when writing a program."'
    const result = window._lscsv.to_map(csvText);
    var key = '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"';
    var expect_ja = '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"'
    var expect_en = '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.'
    var lang_hash = result[key];
    expect(lang_hash['ja']).to.equal(expect_ja);
    expect(lang_hash['en']).to.equal(expect_en);
  });

  it('名前を変える行を正しく解析できるか', function() {
    const csvText = "origin,ja,en\n名前変えるよ,名前変えるよ,I\'ll change the name.";
    const result = window._lscsv.to_map(csvText);
    expect(result['名前変えるよ']['ja']).to.equal('名前変えるよ');
    expect(result['名前変えるよ']['en']).to.equal("I'll change the name.");
  });

  it('変数の値を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番の変数の値は \\V[1] です。,1番の変数の値は \\V[1] です。,The value of variable 1 is \\V[1].';
    const result = window._lscsv.to_map(csvText);
    expect(result['1番の変数の値は \\V[1] です。']['ja']).to.equal('1番の変数の値は \\V[1] です。');
    expect(result['1番の変数の値は \\V[1] です。']['en']).to.equal('The value of variable 1 is \\V[1].');
  });

  it('アクターの名前を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番のアクターの名前は \\N[1] です。,1番のアクターの名前は \\N[1] です。,The name of the 1 actor is \\N[1].';
    const result = window._lscsv.to_map(csvText);
    expect(result['1番のアクターの名前は \\N[1] です。']['ja']).to.equal('1番のアクターの名前は \\N[1] です。');
    expect(result['1番のアクターの名前は \\N[1] です。']['en']).to.equal('The name of the 1 actor is \\N[1].');
  });

  it('パーティーメンバーの名前を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番のパーティーメンバーの名前は \\P[1] です。,1番のパーティーメンバーの名前は \\P[1] です。,The name of party member 1 is \\P[1].';
    const result = window._lscsv.to_map(csvText);
    expect(result['1番のパーティーメンバーの名前は \\P[1] です。']['ja']).to.equal('1番のパーティーメンバーの名前は \\P[1] です。');
    expect(result['1番のパーティーメンバーの名前は \\P[1] です。']['en']).to.equal('The name of party member 1 is \\P[1].');
  });

  it('所持金を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n現在の所持金は \\G です。,現在の所持金は \\G です。,The current amount of money you have is \\G';
    const result = window._lscsv.to_map(csvText);
    expect(result['現在の所持金は \\G です。']['ja']).to.equal('現在の所持金は \\G です。');
    expect(result['現在の所持金は \\G です。']['en']).to.equal('The current amount of money you have is \\G');
  });

  it('色を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nこの文字は \\C[2] 赤色 \\C[0] 通常色 です。,この文字は \\C[2] 赤色 \\C[0] 通常色 です。,This letter is \\C[2] red \\C[0] normal color.';
    const result = window._lscsv.to_map(csvText);
    expect(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['ja']).to.equal('この文字は \\C[2] 赤色 \\C[0] 通常色 です。');
    expect(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['en']).to.equal('This letter is \\C[2] red \\C[0] normal color.');
  });

  it('アイコンを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nこれはアイコン \\I[64] の表示です。,これはアイコン \\I[64] の表示です。,This is the display of the icon \\I[64].';
    const result = window._lscsv.to_map(csvText);
    expect(result['これはアイコン \\I[64] の表示です。']['ja']).to.equal('これはアイコン \\I[64] の表示です。');
    expect(result['これはアイコン \\I[64] の表示です。']['en']).to.equal('This is the display of the icon \\I[64].');
  });

  it('文字サイズを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文字サイズを \\{ 大きく \\} 小さくします。,文字サイズを \\{ 大きく \\} 小さくします。,The font size will be reduced to \\{large \\}smaller.';
    const result = window._lscsv.to_map(csvText);
    expect(result['文字サイズを \\{ 大きく \\} 小さくします。']['ja']).to.equal('文字サイズを \\{ 大きく \\} 小さくします。');
    expect(result['文字サイズを \\{ 大きく \\} 小さくします。']['en']).to.equal('The font size will be reduced to \\{large \\}smaller.');
  });

  it('所持金ウィンドウを表示する行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\$ 所持金ウィンドウを表示します。,\\$ 所持金ウィンドウを表示します。,\\$ Display the money window.';
    const result = window._lscsv.to_map(csvText);
    expect(result['\\$ 所持金ウィンドウを表示します。']['ja']).to.equal('\\$ 所持金ウィンドウを表示します。');
    expect(result['\\$ 所持金ウィンドウを表示します。']['en']).to.equal('\\$ Display the money window.');
  });

  it('文章の途中でウェイトを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中で 1/4 秒間のウェイトを \. します。,文章の途中で 1/4 秒間のウェイトを \. します。,It will display a 1/4 second wait \. in the middle of a sentence.';
    const result = window._lscsv.to_map(csvText);
    expect(result['文章の途中で 1/4 秒間のウェイトを \. します。']['ja']).to.equal('文章の途中で 1/4 秒間のウェイトを \. します。');
    expect(result['文章の途中で 1/4 秒間のウェイトを \. します。']['en']).to.equal('It will display a 1/4 second wait \. in the middle of a sentence.');
  });

  it('文章の途中で1秒間のウェイトを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中で 1 秒間のウェイトを \| します。,文章の途中で 1 秒間のウェイトを \| します。,The text is weighted for one second \| in the middle of a sentence.';
    const result = window._lscsv.to_map(csvText);
    expect(result['文章の途中で 1 秒間のウェイトを \| します。']['ja']).to.equal('文章の途中で 1 秒間のウェイトを \| します。');
    expect(result['文章の途中で 1 秒間のウェイトを \| します。']['en']).to.equal('The text is weighted for one second \| in the middle of a sentence.');
  });

  it('文章の途中でボタン入力待ちを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中でボタンの入力待ちを \! します。,文章の途中でボタンの入力待ちを \! します。,Waiting for button input in the middle of a sentence \! The button is pressed.';
    const result = window._lscsv.to_map(csvText);
    expect(result['文章の途中でボタンの入力待ちを \! します。']['ja']).to.equal('文章の途中でボタンの入力待ちを \! します。');
    expect(result['文章の途中でボタンの入力待ちを \! します。']['en']).to.equal('Waiting for button input in the middle of a sentence \! The button is pressed.');
  });

  it('一瞬で表示する行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\>この行の文字を一瞬で表示します。\\<,\\>この行の文字を一瞬で表示します。\\<,\\>Display the characters of this line in a moment.\\<';
    const result = window._lscsv.to_map(csvText);
    expect(result['\\>この行の文字を一瞬で表示します。\\<']['ja']).to.equal('\\>この行の文字を一瞬で表示します。\\<');
    expect(result['\\>この行の文字を一瞬で表示します。\\<']['en']).to.equal('\\>Display the characters of this line in a moment.\\<');
  });

  it('入力待ちを行わない行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\^文章表示後の入力待ちを行いません。,\\^文章表示後の入力待ちを行いません。,\\^Do not wait for input after displaying a sentence.';
    const result = window._lscsv.to_map(csvText);
    expect(result['\\^文章表示後の入力待ちを行いません。']['ja']).to.equal('\\^文章表示後の入力待ちを行いません。');
    expect(result['\\^文章表示後の入力待ちを行いません。']['en']).to.equal('\\^Do not wait for input after displaying a sentence.');
  });

  it('バックスラッシュを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nバックスラッシュの表示は \\\\ です。,バックスラッシュの表示は \\\\ です。,The backslash is displayed at \\\\';
    const result = window._lscsv.to_map(csvText);
    expect(result['バックスラッシュの表示は \\\\ です。']['ja']).to.equal('バックスラッシュの表示は \\\\ です。');
    expect(result['バックスラッシュの表示は \\\\ です。']['en']).to.equal('The backslash is displayed at \\\\');
  });

  it('複合させた行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"複合させます\n\\{\\C[2]\\N[2]\\I[22]","複合させます\n\\{\\C[2]\\N[2]\\I[22]","Compounding\n\\{\\C[2]\\N[2]\\I[22]"';
    const result = window._lscsv.to_map(csvText);
    expect(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['ja']).to.equal('複合させます\n\\{\\C[2]\\N[2]\\I[22]');
    expect(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['en']).to.equal('Compounding\n\\{\\C[2]\\N[2]\\I[22]');
  });
});

describe('Langscore', function() 
{
  before(() => initializeRPGMaker());

  this.beforeEach(() =>{
    window._langscore.changeLanguage("en", true);
  });

  afterEach(function() {
  });

  it('テキストを正しく翻訳すること', function() 
  {
    const testMap = new Map();
    let createTranslateMap = (jaText, enText) =>
    {
      let translates = new Map();
      translates["ja"] = jaText;
      translates["en"] = enText;
      testMap[jaText] = translates;
    };
    createTranslateMap("通常のテキストです", "Normal text.");
    createTranslateMap("改行を含む\nテキストです", "Includes line breaks\nText.");
    createTranslateMap(
      "\"\"タ\"\"フ\"\"ルクォーテーションを含むテキストです\"\"",
      "The text contains a t\"\"ouh\"\"le quotation."
    );

    testMap.forEach((value, key) => 
    {
      var inText   = value.ja;
      var outText  = value.en;
      const result = window._langscore.translate(inText, testMap, "en");
      expect(result).to.equal(outText);
    });
  });

  it('翻訳が利用できない場合、元のテキストを返すこと', function() {
    const testMap = {
      "Hello": { "ja": "こんにちは" },
    };
    const result = window._langscore.translate("Goodbye", testMap, "ja");
    expect(result).to.equal("Goodbye");
  });

  it('翻訳文から原文テキストを正しく取得すること', function() {
    const testMap = {
      "Hello": { "ja": "こんにちは", "en": "Hello" },
      "Goodbye": { "ja": "さようなら" }
    };
    const result = window._langscore.fetch_original_text("こんにちは", testMap);
    expect(result).to.equal("Hello");
  });

  it('言語の変更が行えている', function() 
  {
    expect(window.Langscore.langscore_current_language).to.equal('en');
    
    window._langscore.changeLanguage("ja", true);
    expect(window.Langscore.langscore_current_language).to.equal('ja');
  });

  it('画像ファイルを正しくロードすること', function() 
  {
    if(window.StorageManager.isLocalMode())
    {
      //画像パスの自動置き換え
      const path = window.ImageManager.loadBitmap('img/pictures/', 'nantoka8').url;
      expect(path).to.equal('img/pictures/nantoka8_en.png');
    }
    {
      //CSVから画像パスを読み込み
      const path = window.ImageManager.loadBitmap('img/pictures/', 'nantoka10').url;
      expect(path).to.equal('img/pictures/nantoka8.png');
    }
  });

  it('言語を正しく変更すること', function() {
    window._langscore.changeLanguage("en");
    expect(window.Langscore.langscore_current_language).to.equal("en");
  });

  it('セーブデータが正しく保存されること', function() 
  {    
    {
      window._langscore.changeLanguage("ja");
      
      const saveContents = window.DataManager.makeSaveContents();

      // セーブデータが正しく保存されているかの確認
      expect(saveContents.actors.actor(1).name()).to.equal('エルーシェ');
      expect(saveContents.actors.actor(1).nickname()).to.equal('雑用係');
      expect(saveContents.actors.actor(1).profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
      expect(saveContents.actors.actor(2).name()).to.equal('ラフィーナ');
      expect(saveContents.actors.actor(2).nickname()).to.equal('傲慢ちき');
      expect(saveContents.actors.actor(2).profile()).to.equal('チビのツンデレウーマン。\n魔法が得意。');
    }
    
    {
      window._langscore.changeLanguage("en");

      // セーブデータの作成
      const saveContents = window.DataManager.makeSaveContents();
      
      // セーブデータが正しく保存されているかの確認
      expect(saveContents.actors.actor(1).name()).to.equal('エルーシェ');
      expect(saveContents.actors.actor(1).nickname()).to.equal('雑用係');
      expect(saveContents.actors.actor(1).profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
      expect(saveContents.actors.actor(2).name()).to.equal('ラフィーナ');
      expect(saveContents.actors.actor(2).nickname()).to.equal('傲慢ちき');
      expect(saveContents.actors.actor(2).profile()).to.equal('チビのツンデレウーマン。\n魔法が得意。');
    }
  });
  
  it('アクター名が正しく更新されること', function() 
  {
    window._langscore.changeLanguage("ja", true);
    const actor = window.$gameActors.actor(1);
    expect(actor.name()).to.equal("エルーシェ");
    expect(actor.nickname()).to.equal("雑用係");
    expect(actor.profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');

    window._langscore.changeLanguage("en", true);
    expect(actor.name()).to.equal("eluche");
    expect(actor.nickname()).to.equal("Compassionate");
    expect(actor.profile()).to.equal(`Kestinius' scullery maid.\nThere is not that much work.`);

    window._langscore.changeLanguage("ja", true);
    expect(actor.name()).to.equal("エルーシェ");
    expect(actor.nickname()).to.equal("雑用係");
    expect(actor.profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
  });

  it('スキルが正しく更新されること', function() 
  {

    window._langscore.changeLanguage("ja", true);
    window.$dataSkills.forEach(skill => {
      if(skill === null){ return; }
      expect(testSkills.includes(item => item.ja === skill.name)).to.exist;
      expect(testSkills.includes(item => item.ja === skill.description)).to.exist;
      expect(testSkills.includes(item => item.ja === skill.message1)).to.exist;
      expect(testSkills.includes(item => item.ja === skill.message2)).to.exist;
    });

    window._langscore.changeLanguage("en", true);
    window.$dataSkills.forEach(skill => {
      if(skill === null){ return; }
      expect(testSkills.includes(item => item.en === skill.name)).to.exist;
      expect(testSkills.includes(item => item.en === skill.description)).to.exist;
      expect(testSkills.includes(item => item.en === skill.message1)).to.exist;
      expect(testSkills.includes(item => item.en === skill.message2)).to.exist;
    });

  });

});



describe('Langscore for Map', function() 
{

  before(async () => {
    await initializeRPGMaker();
    var mapID = 1;
    window.DataManager.loadMapData(mapID);
    await waitForCondition(() => window.DataManager.isMapLoaded() === true);
    
    window.$gameMap.setup(mapID);
    window.$gameMap._interpreter._mapId = mapID;
  });

  it('翻訳文を正しく取得できるか', function() 
  {
    window._langscore.changeLanguage("en", true);

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      const result = window._langscore.translate_for_map(inText);
      expect(result).to.equal(outText);
    });
  });

  
  it('テキストの変換に成功するか', function() 
  {
    window._langscore.changeLanguage("en", true);

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });

  
  it('ショップが翻訳されているか', function() 
  {
    var evList = window.$gameMap.events();
    var ev = evList.filter((event) => { return event.eventId() == 6; })[0];
    ev.start();
    expect(window.$gameMap.setupStartingMapEvent()).to.true;
  });

  it('セーブファイルの書き込みとロード', function() 
  {
    window._langscore.changeLanguage("ja", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("エルーシェ");
    }

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.ja;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
    //日本語を指定した状態でセーブ
    window.DataManager.saveGame(1);

    //英語に変更
    window._langscore.changeLanguage("en", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });

    //日本語を指定したときのデータをロード
    window.DataManager.loadGame(1);

    //言語設定はセーブデータに影響を受けないので、英語になっていることが正しい。
    
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });

  
  it('2024/06報告不具合', function() 
  {
    //ex.)ja -> enへの言語変更後にセーブを行うと、一部の制御文字を使用した
    //    文章が日本語で表示される。期待する言語は英語。
    window._langscore.changeLanguage("ja", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("エルーシェ");
    }

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.ja;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });

    //英語に変更
    window._langscore.changeLanguage("en", true);
    //日本語を指定した状態でセーブ
    window.DataManager.saveGame(1);

    //不具合が発生する場合、一部が日本語になるためtext.enと一致しない。
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });

});