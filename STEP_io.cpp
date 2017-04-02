// コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

void DumpShape(const TopoDS_Shape& sh, int depth = 0);
void CheckDoc(Handle(TDocStd_Document)& doc);

bool OpenSTEP(const char* filename)
{
	STEPCAFControl_Reader reader;
	if (reader.ReadFile(filename) != IFSelect_RetDone)
		return false;

	Handle(TDocStd_Document) doc = new TDocStd_Document("MDTV-XCAF");
	if (!reader.Transfer(doc))
		return false;

	//cout << "IsXFAFDoc = " << XCAFDoc_DocumentTool::IsXCAFDocument(doc) << endl;
	if (!XCAFDoc_DocumentTool::IsXCAFDocument(doc))
		return false;		// cannot be handled as XCAF doument

	CheckDoc(doc);
	return true;
}

void CheckDoc(Handle(TDocStd_Document)& doc)
{
	TDF_Label lblMain = doc->Main();
	cout << "IsRoot = " << lblMain.IsRoot() << ", IsImported = " << lblMain.IsImported() << endl;

	// Find XCAFDoc_DocumentTool::ShapesLabel below lblMain, and create ShapeTool to handle shapes in the STEP file.
	Handle(XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(lblMain);

	// doesn't work
	//TopoDS_Shape sh = myAssembly->GetShape(myAssembly->BaseLabel());
	//cout << "sh.IsNull = " << sh.IsNull() << endl;
	//cout << "sh.ShapeType = " << sh.ShapeType() << endl;

	TDF_LabelSequence labels;
	myAssembly->GetShapes(labels);
	cout << "labels.Length = " << labels.Length() << endl;
	for (int i = 1; i <= labels.Length(); i++)
	{
		const TDF_Label& lbl = labels.Value(i);
		cout << "labels[" << i << "]" << endl;
		cout << " shape = " << XCAFDoc_ShapeTool::IsShape(lbl);
		cout << " simple = " << XCAFDoc_ShapeTool::IsSimpleShape(lbl);
		cout << " sub = " << XCAFDoc_ShapeTool::IsSubShape(lbl);
		cout << " free = " << XCAFDoc_ShapeTool::IsFree(lbl) << endl;
		cout << " ref = " << XCAFDoc_ShapeTool::IsReference(lbl);
		cout << " component = " << XCAFDoc_ShapeTool::IsComponent(lbl);
		cout << " compound = " << XCAFDoc_ShapeTool::IsCompound(lbl);
		cout << " extern = " << XCAFDoc_ShapeTool::IsExternRef(lbl) << endl;
		cout << " top = " << myAssembly->IsTopLevel(lbl) << endl;

		//TopoDS_Shape sh = XCAFDoc_ShapeTool::GetShape(lbl);
		//cout << "labels[" << i << "] shape type = " << sh.ShapeType() << endl;

		if (!XCAFDoc_ShapeTool::IsFree(lbl))
			continue;
		TopoDS_Shape sh = XCAFDoc_ShapeTool::GetShape(lbl);
		cout << "labels[" << i << "] shape type = " << sh.ShapeType() << endl;
		DumpShape(sh);
		//if (sh.ShapeType() == TopAbs_COMPOUND)
		//{
		//	TopExp_Explorer ex;
		//	for (ex.Init(sh, TopAbs_SOLID); ex.More(); ex.Next())
		//	{
		//		ex.Current();
		//	}
		//}

		cout << endl;
	}
}

void PrintDepth(int d)
{
	while (d--)
		cout << "  ";
}

const char* GetShapeTypeName(TopAbs_ShapeEnum type)
{
	switch (type)
	{
		case TopAbs_COMPOUND:	return "COMPOUND";
		case TopAbs_COMPSOLID:	return "COMPSOLID";
		case TopAbs_SOLID:		return "SOLID";
		case TopAbs_SHELL:		return "SHELL";
		case TopAbs_FACE:		return "FACE";
		case TopAbs_WIRE:		return "WIRE";
		case TopAbs_EDGE:		return "EDGE";
		case TopAbs_VERTEX: 	return "VERTEX";
		case TopAbs_SHAPE:		return "SHAPE";
		default:				return "(unknown)";
	}
}

const char* GetSurfaceTypeName(GeomAbs_SurfaceType type)
{
	switch (type)
	{
		case GeomAbs_Plane: 				 return "Plane";
		case GeomAbs_Cylinder:				 return "Cylinder";
		case GeomAbs_Cone:					 return "Cone";
		case GeomAbs_Sphere:				 return "Sphere";
		case GeomAbs_Torus: 				 return "Torus";
		case GeomAbs_BezierSurface: 		 return "BezierSurface";
		case GeomAbs_BSplineSurface:		 return "BSplineSurface";
		case GeomAbs_SurfaceOfRevolution:	 return "SurfaceOfRevolution";
		case GeomAbs_SurfaceOfExtrusion:	 return "SurfaceOfExtrusion";
		case GeomAbs_OffsetSurface: 		 return "OffsetSurface";
		case GeomAbs_OtherSurface:			 return "OtherSurface";
		default:							return "(unknown)";
	}
}

const char* GetOrientationName(TopAbs_Orientation ori)
{
	switch (ori)
	{
		case TopAbs_FORWARD:	return "FORWARD";
		case TopAbs_REVERSED:	return "REVERSED";
		case TopAbs_INTERNAL:	return "INTERNAL";
		case TopAbs_EXTERNAL:	return "EXTERNAL";
		default:				return "(unknown)";
	}
}

void DumpShape(const TopoDS_Shape& sh, int depth)
{
	if (sh.ShapeType() == TopAbs_VERTEX)
	{
		PrintDepth(depth); cout << GetShapeTypeName(sh.ShapeType()) << endl;
		//<< " = []" << endl;
	}
	else if (sh.ShapeType() == TopAbs_EDGE)		// two vertices in an edge
	{
		PrintDepth(depth); cout << GetShapeTypeName(sh.ShapeType());
		cout << "(" << GetOrientationName(sh.Orientation()) << ") [ ";
		int vcount = 0;
		TopoDS_Iterator it;
		for (it.Initialize(sh); it.More(); it.Next())
		{
			const TopoDS_Shape& s = it.Value();
			assert(s.ShapeType() == TopAbs_VERTEX);
			//cout << GetShapeTypeName(s.ShapeType()) << "[] ";
			vcount++;
		}
		const TopoDS_Edge& edge = TopoDS::Edge(sh);
		gp_Pnt p1 = BRep_Tool::Pnt(TopExp::FirstVertex(edge));
		gp_Pnt p2 = BRep_Tool::Pnt(TopExp::LastVertex(edge));
		//PrintDepth(depth);
		cout << "VERTEX1(" << p1.X() << ", " << p1.Y() << ", " << p1.Z() << ") ";
		if (sh.Orientation() == TopAbs_REVERSED)
			cout << "<-- ";
		else if (sh.Orientation() == TopAbs_FORWARD)
			cout << "--> ";
		else if (vcount > 2)
			cout << "... ";
		cout << "VERTEX" << vcount << "(" << p2.X() << ", " << p2.Y() << ", " << p2.Z() << ") ";
		cout << "]" << endl;
	}
	else if (sh.ShapeType() == TopAbs_WIRE)
	{
		PrintDepth(depth); cout << GetShapeTypeName(sh.ShapeType()) << " [" << endl;
		BRepTools_WireExplorer wex;
		for (wex.Init(TopoDS::Wire(sh)); wex.More(); wex.Next())
		{
			DumpShape(wex.Current(), depth + 1);
		}
		PrintDepth(depth); cout << "]" << endl;
	}
	else if (sh.ShapeType() == TopAbs_FACE)
	{
		const TopoDS_Face& face = TopoDS::Face(sh);
		//TopLoc_Location L;
		//GeomAdaptor_Surface surf(BRep_Tool::Surface(face, L));
		// Include face location into gp object parameters.
		GeomAdaptor_Surface surf(BRep_Tool::Surface(face));

		PrintDepth(depth);
		cout << GetShapeTypeName(sh.ShapeType());
		cout << "(" << GetSurfaceTypeName(surf.GetType()) << ")";
		cout << "(" << GetOrientationName(sh.Orientation()) << ")" << endl;

		if (surf.GetType() == GeomAbs_Sphere)
		{
			gp_Sphere sphere = surf.Sphere();
			const gp_Pnt& pnt = sphere.Location();
			PrintDepth(depth); cout << ".Location = (" << pnt.X() << ", " << pnt.Y() << ", " << pnt.Z() << "), " << endl;
			PrintDepth(depth); cout << ".Radius = " << sphere.Radius() << endl;
		}

		PrintDepth(depth);
		cout << "[" << endl;
		TopoDS_Iterator it;
		for (it.Initialize(sh); it.More(); it.Next())
		{
			DumpShape(it.Value(), depth + 1);
		}
		PrintDepth(depth); cout << "]" << endl;
	}
	else
	{
		PrintDepth(depth); cout << GetShapeTypeName(sh.ShapeType()) << " [" << endl;
		TopoDS_Iterator it;
		for (it.Initialize(sh); it.More(); it.Next())
		{
			DumpShape(it.Value(), depth + 1);
		}
		PrintDepth(depth); cout << "]" << endl;
	}
}

bool MakeSTEP(const std::string& filename)
{
	TopoDS_Shape sph1 = BRepPrimAPI_MakeSphere(gp_Pnt(4, 4, 4), 5).Shape();
	TopoDS_Shape sph2 = BRepPrimAPI_MakeSphere(gp_Pnt(0, 0, 0), 5).Shape();

	TopoDS_Shape S = BRepAlgoAPI_Fuse(sph1, sph2);
	//TopoDS_Compound()

	DumpShape(S);

	STEPCAFControl_Writer writer;//(XSDRAW::Session(),Standard_False);
	writer.SetColorMode(Standard_True);
	writer.SetNameMode(Standard_True);

	Handle(TDocStd_Document) doc = new TDocStd_Document("MDTV-XCAF");
	Handle (XCAFDoc_ShapeTool) myAssembly = XCAFDoc_DocumentTool::ShapeTool(doc->Main());
	//TDF_Label aLabel = myAssembly->NewShape();
	TDF_Label aLabel = myAssembly->AddShape(S, Standard_True);

	CheckDoc(doc);

	if (!writer.Transfer(doc))
		return false;
	if (writer.Write(filename.c_str()) != IFSelect_RetDone)
		return false;
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::string TestModelDir = "C:\\Users\\khara\\dev\\STEP_io\\TestModel\\";

	// Write Test
	{
		auto outFile = TestModelDir + "out.step";
		MakeSTEP(outFile);
	}

	const char* files[] = {
		//"box_1.step",
		//"cylinder_1.step",
		//"sphere_1.step",
		//"sphere+sphere_1.step",
		//"sphere+sphere_1-0.step",
		//"sph+sph_1.step",		// Generated by Autodesk 123D Design
		//"torus_1.step",
		//"cone+sphere_1.step",
		"out.step",
	};

	// no effect for "sphere+sphere_1.step"
	//STEPControl_Controller::Init();
	//assert(Interface_Static::IVal("read.step.product.mode") == 1);
	//if (!Interface_Static::SetIVal("read.step.product.mode", 0))		// use regacy mode
	//	return false;
	//assert(Interface_Static::IVal("read.step.product.mode") == 0);

	for (auto* filename : files)
	{
		std::string TestFile = TestModelDir + filename;

		std::cout << endl << endl << filename << std::endl;
		OpenSTEP(TestFile.c_str());
	}

	cout << "Press a key..."; getchar();
	return 0;
}
