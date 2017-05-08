Registration Method Plugin: Centerline {#org_custusx_registration_method_centerline}
===========================================================

\addindex org_custusx_registration_method_centerline_widget
Centerline Registration {#org_custusx_registration_method_centerline_widget}
===========================================================

Registration method to register a centerline (.vtk file) to a path acquired from a tracked tool.
Translation along each axis (x,y,z) and rotation around each axis (x,y,z) is optional.

Intended use: Endoscopy in esophagus (local/updated registration during procedure).

Tips: Switch off z translation and z rotation to use the registration method for local corrections
in navigated endoscopy in esophagus.

Input: centerline .vtk file + tracking data from tool

Output: Updated image to patient registration