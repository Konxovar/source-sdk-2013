// Created by Crowbar 0.71

$modelname "props_xen\c4a2c\introrock1.mdl"

$bodygroup "body"
{
	studio "c4a2c_chase_rock_03.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition -20.059 3.631 -75.321

$cdmaterials "models\props_xen\rocks\"

$cbox 0 0 0 0 0 0

$bbox -157.872 -155.01 -223.882 150.611 114.891 73.239


$sequence "idle_before" {
	"introrock1_anims\idle_before.smd"
	fadein 0.2
	fadeout 0.2
	fps 1
	loop
}

$collisionmodel "introrock1_physics.smd"
{
	$mass 10302
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 3

}
