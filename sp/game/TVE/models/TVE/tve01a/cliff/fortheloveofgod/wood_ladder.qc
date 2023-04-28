// Created by Crowbar 0.71

$modelname "\TVE\tve01a\cliff\wood_ladder.mdl"

$staticprop

$bodygroup "wood_ladder"
{
	studio "wood_ladder.smd"
}


$surfaceprop "Wood_Solid"

$contents "solid"

$illumposition -3 -0.5 0.03

$cdmaterials "models\TVE\tve01a\cliff\"

$cbox 0 0 0 0 0 0

$bbox -2.25 -16.25 -128.25 3.25 10.25 128.31

$definebone "static_prop" "" 0 0 0 0 0 0 0 0 0 0 0 0


$sequence "idle" {
	"wood_ladder_anims\idle.smd"
	activity "ACT_IDLE" -1
	fadein 0.2
	fadeout 0.2
	fps 1
}

$collisionmodel "wood_ladder_physics.smd"
{
	$mass 25
	$inertia 1
	$damping 0
	$rotdamping 1
	$rootbone " "
	$concave
	$maxconvexpieces 19

}
