{
    gbname = $3
    gbrename = $3
    gsub (/GxB_/, "GxM_", gbrename) ;
    gsub (/GrB_/, "GrM_", gbrename) ;
    gsub (/GB_/, "GM_", gbrename) ;
    if (length (gbname) > 0) {
        printf "#define %s %s\n", gbname, gbrename
    }
}
