void add_f3d_logo_to_drawlist(ImDrawList* drawlist, const float x, const float y, const float s, const ImU32 color, const float lw){
    const ImVec2 pnts0[4] = { {x+s*2.f, y+s*-4.5f}, {x+s*-2.f, y+s*0.5f}, {x+s*-2.f, y+s*-1.5f}, {x+s*2.f, y+s*-4.5f} };
    drawlist->AddPolyline(pnts0, 4, color, 0, lw);
    drawlist->AddConvexPolyFilled(pnts0, 4, color);
    const ImVec2 pnts1[4] = { {x+s*-0.f, y+s*-0.5f}, {x+s*2.f, y+s*1.5f}, {x+s*2.f, y+s*-0.5f}, {x+s*-0.f, y+s*-0.5f} };
    drawlist->AddPolyline(pnts1, 4, color, 0, lw);
    drawlist->AddConvexPolyFilled(pnts1, 4, color);
    const ImVec2 pnts2[5] = { {x+s*0.4f, y+s*-0.5f}, {x+s*2.f, y+s*-4.5f}, {x+s*-2.f, y+s*-1.5f}, {x+s*-2.f, y+s*0.5f}, {x+s*-0.4f, y+s*0.5f} };
    drawlist->AddPolyline(pnts2, 5, color, 0, lw);
    const ImVec2 pnts3[5] = { {x+s*-0.f, y+s*-0.5f}, {x+s*-2.f, y+s*4.5f}, {x+s*2.f, y+s*1.5f}, {x+s*2.f, y+s*-0.5f}, {x+s*-0.f, y+s*-0.5f} };
    drawlist->AddPolyline(pnts3, 5, color, 0, lw);
    drawlist->AddCircleFilled({x+s*0.4f, y+s*-0.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*2.f, y+s*1.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*-2.f, y+s*4.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*-0.f, y+s*-0.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*2.f, y+s*-0.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*-2.f, y+s*0.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*2.f, y+s*-4.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*-2.f, y+s*-1.5f}, lw/2, color);
    drawlist->AddCircleFilled({x+s*-0.4f, y+s*0.5f}, lw/2, color);
}
