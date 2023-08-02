        function colors_slider_get_color(rate)
        {
            var rs = [0xFF,0xF0,0x00,0x00,0x00,0xFF,0xFF];
            var gs = [0x00,0xF0,0xFF,0xFF,0x00,0x00,0x00];
            var bs = [0x00,0x00,0x00,0xFF,0xFF,0xFF,0x00];

            let get_rgb = (r,g,b,left,right,rate) => (
                  (Math.round(r[left] + (r[right] - r[left]) * rate) << 16)
                + (Math.round(g[left] + (g[right] - g[left]) * rate) << 8)
                + (Math.round(b[left] + (b[right] - b[left]) * rate) << 0)
            );

            rate = Math.abs(rate);

            if(rate < 0.17)
            {
                return get_rgb(rs,gs,bs,0,1,(rate - 0.0) / 0.17);
            }
            if(rate < 0.33)
            {
                return get_rgb(rs,gs,bs,1,2,(rate - 0.17) / 0.16);
            }
            if(rate < 0.5)
            {
                return get_rgb(rs,gs,bs,2,3,(rate - 0.33) / 0.17);
            }
            if(rate < 0.66)
            {
                return get_rgb(rs,gs,bs,3,4,(rate - 0.5) / 0.16);
            }
            if(rate < 0.83)
            {
                return get_rgb(rs,gs,bs,4,5,(rate - 0.66) / 0.17);
            }

            return get_rgb(rs,gs,bs,5,6,(rate - 0.83) / 0.17);
        }

        function dec_to_rgb(dec)
        {
            let str = dec.toString(16);
            let len = str.length;

            for(let i = 0; i < 6 - len; i++)
            {
                str = "0" + str;
            }

            return "#"+str;
        }

        var slider_rgb = 0xFF0000;
        var plane_rgb = 0xFFFFFF;

        var slider_rate_y = 1.0;
        var plane_rate_x = 0.0;
        var plane_rate_y = 0.0;

        var plane_point_drag = 0;

        function get_rgb_by_rate(rgb1,rgb2,rate)
        {
            let r1 = (rgb1 & 0xFF0000) >> 16;
            let r2 = (rgb2 & 0xFF0000) >> 16;
            let g1 = (rgb1 & 0x00FF00) >> 8;
            let g2 = (rgb2 & 0x00FF00) >> 8;
            let b1 = (rgb1 & 0x0000FF);
            let b2 = (rgb2 & 0x0000FF);

            return (Math.round(r1 + (r2 - r1) * rate) << 16)
                + (Math.round(g1 + (g2 - g1) * rate) << 8)
                + (Math.round(b1 + (b2 - b1) * rate) << 0);
        }

        function color_display_update()
        {
            let color_text= document.getElementById("color_now");
            let color_block = document.getElementById("color_display_block");

            color_text.text = dec_to_rgb(plane_rgb);
            color_block.style.backgroundColor = dec_to_rgb(plane_rgb);
        }

        function plane_move(event) {
            event.preventDefault();

            if(plane_point_drag == 0)
                return ;

            let mask = document.getElementById("colors_plane_mask");
            let point = document.getElementById("colors_plane_point");

            let wid_h = point.offsetWidth / 2;
            let hei_h = point.offsetHeight / 2;

            //console.log("id:"+mask.id+" h:"+hei_h+" w:"+wid_h);

            let x = 0;
            let y = 0;

            if(event.targetTouches){
                x = event.targetTouches[0].clientX;
                y = event.targetTouches[0].clientY;
            }
            else{
                x = event.clientX;
                y = event.clientY;
            }
            let mask_bind_client = mask.getBoundingClientRect();
            let p_x1 = mask_bind_client.left;
            let p_x2 = p_x1 + mask_bind_client.width;
            let p_y1 = mask_bind_client.top;
            let p_y2 = p_y1 + mask_bind_client.height;

            let p_wid_h = mask.offsetWidth / 2;
            let p_hei_h = mask.offsetHeight / 2;

            if(x < p_x1)
            {
                x = p_x1 ;
            }
            else if(x > p_x2)
            {
                x = p_x2 ;
            }

            if(y  < p_y1 )
            {
                y = p_y1;
            }
            else if(y > p_y2)
            {
                y = p_y2;
            }

            point.style.left = (x - p_x1) + "px";
            point.style.top = (y - p_y1) + "px";

            let rate_x = (x - p_x1) / mask_bind_client.width;
            let rate_y = (y - p_y1) / mask_bind_client.height;

            let rgb_tmp = get_rgb_by_rate(0xFFFFFF, slider_rgb, rate_x);
            plane_rgb = get_rgb_by_rate(rgb_tmp, 0, rate_y);
            plane_rate_x = rate_x;
            plane_rate_y = rate_y;
            color_display_update();
            //console.log("now rgb: 0x" + plane_rgb.toString(16));
        }

        function event_plane_down(event)
        {
            plane_point_drag = 1;
            plane_move(event);
        }
        function event_plane_up(event)
        {
            plane_point_drag = 0;
        }

        var slider_point_drag = 0;


        function slider_move(event) {
            event.preventDefault();

            if(slider_point_drag == 0)
                return ;

            let plane = document.getElementById("colors_plane");
            let slider = document.getElementById("colors_slider_base");
            let point = document.getElementById("colors_slider_point");

            let hei_h = point.offsetHeight / 2;

            let y = 0;

            if(event.targetTouches){
                y = event.targetTouches[0].clientY;
            }
            else{
                y = event.clientY;
            }
            let slider_bind_client = slider.getBoundingClientRect();
            let p_y1 = slider_bind_client.top;
            let p_y2 = p_y1 + slider_bind_client.height;

            let p_hei_h = slider.offsetHeight / 2;

            if(y  < p_y1 )
            {
                y = p_y1;
            }
            else if(y > p_y2)
            {
                y = p_y2;
            }

            point.style.top = (y - p_y1) + "px";

            let rate_y = (y - p_y1) / slider_bind_client.height;
            slider_rgb = colors_slider_get_color(rate_y);
            slider_rate_y = rate_y;

            let rgb_tmp = get_rgb_by_rate(0xFFFFFF, slider_rgb, plane_rate_x);
            plane_rgb = get_rgb_by_rate(rgb_tmp, 0, plane_rate_y);

            color_display_update();
            plane.style.backgroundColor = dec_to_rgb(slider_rgb);
            //console.log("now rgb: 0x" + plane_rgb.toString(16));
            //console.log("rate: " + rate_y + " color: " + dec_to_rgb(slider_rgb));
        }

        function event_slider_down(event)
        {
            slider_point_drag = 1;
            slider_move(event);
        }
        function event_slider_up(event)
        {
            slider_point_drag = 0;
        }

        function color_get_select_rgb()
        {
            return plane_rgb;
        }

        function color_selector_event_reg()
        {
            document.getElementById("colors_plane_mask").addEventListener("touchstart", event_plane_down);
            document.getElementById("colors_plane_mask").addEventListener("touchend",   event_plane_up);
            document.getElementById("colors_plane_mask").addEventListener("touchmove",  plane_move);

            document.getElementById("colors_plane_mask").addEventListener("mousedown",  event_plane_down);
            document.getElementById("colors_selector").addEventListener("mouseup",    event_plane_up);
            document.getElementById("colors_selector").addEventListener("mousemove",  plane_move);
            document.body.addEventListener("mouseup",    event_plane_up);
            document.body.addEventListener("mouseleave", event_plane_up);
            document.body.addEventListener("mousemove",  plane_move);

            document.getElementById("colors_slider_base").addEventListener("touchstart", event_slider_down);
            document.getElementById("colors_slider_base").addEventListener("touchend",   event_slider_up);
            document.getElementById("colors_slider_base").addEventListener("touchmove",  slider_move);

            document.getElementById("colors_slider_base").addEventListener("mousedown",  event_slider_down);
            document.getElementById("colors_selector").addEventListener("mouseup",    event_slider_up);
            document.getElementById("colors_selector").addEventListener("mousemove",  slider_move);
            document.body.addEventListener("mouseup",    event_slider_up);
            document.body.addEventListener("mouseleave", event_slider_up);
            document.body.addEventListener("mousemove",  slider_move);
        }

export {color_selector_event_reg, color_get_select_rgb}
