var control_callback =
{
    'log':console.log
}

function reg_control_log(func) {
    control_callback['log'] = func;
}

function control_log(text) {
    control_callback['log'](text);
}

/**
 *
 * @param {*} dom 		dom obj
 * @param {*} left 		control div left
 * @param {*} top 		control div top
 * @param {*} width 	control div width
 * @param {*} weight 	control div height
 * @param {*} move_x 	x range can move
 * @param {*} move_y 	y range can move
 * @param {*} color		ctl point color
 */
function set_ctl_div_style(dom, left, top, width, height, move_x, move_y, color)
{
	if(dom.className != "ctl-div")
	{
		console.log("error class name" + dom.className + ", in set_ctl_div_style dom");
		return ;
	}

	let slot = dom.children[0];
	if(slot.className != "ctl-slot")
	{
		console.log("error class name" + slot.className + ", in set_ctl_div_style slot");
		return ;
	}

	let point = slot.children[0];
	if(point.className != "ctl-point")
	{
		console.log("error class name" + point.className + ", in set_ctl_div_style point");
		return ;
	}

	dom.style.left = left + 'px';
	dom.style.top = top + 'px';
	dom.style.width = width + 'px';
	dom.style.height = height + 'px';

	slot.style.width = move_x + 'px';
	slot.style.height = move_y + 'px';

	let ctl_point_size = Math.min(width, height);

	point.style.height = ctl_point_size + 'px';
	point.style.width = ctl_point_size + 'px';
	point.style.backgroundColor = color;
}

function ctl_poi_move(event, func_ctl_para_set){
	event.preventDefault();

	let current = event.currentTarget;
	let parent = current.parentNode;

	let wid_h = current.offsetWidth / 2;
	let hei_h = current.offsetHeight / 2;

    control_log("id:"+current.id+" h:"+hei_h+" w:"+wid_h);

    let x = 0;
    let y = 0;

	if(event.targetTouches){
		x = event.targetTouches[0].clientX - wid_h;
		y = event.targetTouches[0].clientY - hei_h;
	}
	else{
		x = event.clientX - wid_h;
		y = event.clientY - hei_h;
	}
	let p_x1 = parent.offsetLeft;
	let p_x2 = p_x1 + parent.offsetWidth;
	let p_y1 = parent.offsetTop;
	let p_y2 = p_y1 + parent.offsetHeight;

	let p_wid_h = parent.offsetWidth / 2;
	let p_hei_h = parent.offsetHeight / 2;

	if(x + wid_h < p_x1)
	{
		x = p_x1 - wid_h;
	}
	else if(x + wid_h > p_x2)
	{
		x = p_x2 - wid_h;
	}

	if(y + hei_h < p_y1 )
	{
		y = p_y1 - hei_h;
	}
	else if(y + hei_h > p_y2)
	{
		y = p_y2 - hei_h;
	}

	current.style.left = x + "px";
	current.style.top = y + "px";

	let ctl_val_x = 0;
	let ctl_val_y = 0;

	if(0 != p_wid_h)
		ctl_val_x = ((x+wid_h) - (p_x2 + p_x1)/2) * 100 / p_wid_h;

	if(0 != p_hei_h)
		ctl_val_y = ((y+hei_h) - (p_y2 + p_y1)/2) * 100 / p_hei_h;

	func_ctl_para_set(current, ctl_val_x, ctl_val_y);
}

function ctl_poi_end(event, func_ctl_para_set){
	event.preventDefault();

	let point = event.currentTarget;
	let slot = point.parentNode;
	let div = slot.parentNode;

	if(div.getAttribute("ctl_keep") == "yes")
	{
		return ;
	}

	let p_x1 = slot.offsetLeft;
	let p_x2 = p_x1 + slot.offsetWidth;
	let p_y1 = slot.offsetTop;
	let p_y2 = p_y1 + slot.offsetHeight;

	let l = (p_x1 + p_x2 - point.offsetWidth) / 2;
	let r = (p_y1 + p_y2 - point.offsetHeight) / 2;

	point.style.left = l + "px";
	point.style.top = r + "px";

	func_ctl_para_set(point, 0, 0);
}

export { set_ctl_div_style, ctl_poi_move, ctl_poi_end, reg_control_log }