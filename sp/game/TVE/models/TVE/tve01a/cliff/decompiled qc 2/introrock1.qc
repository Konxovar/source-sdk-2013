// Created by Crowbar 0.71

$modelname "maps\xen_cliff_2.mdl"
$scale 1.1
$bodygroup "body"
{
	studio "xen_cliff_1.smd"
}


$surfaceprop "default"

$contents "solid"

$illumposition -20.059 3.631 -75.321

$cdmaterials "models\maps\rock\cliff"

$cbox 0 0 0 0 0 0

$bbox -157.872 -155.01 -223.882 150.611 114.891 73.239


$sequence "idle" {
	"anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 1
	loop
}

$collisionmodel "xen_cliff_1_phys.smd"
{
	$mass 10302
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave
	$maxconvexpieces 3

}
