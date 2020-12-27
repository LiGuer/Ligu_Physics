#include <stdio.h>#include "LiGu_GRAPHICS/Mat.h"#include "LiGu_GRAPHICS/Plot.h"#include "Tensor.h"/*----------------[ ComputationalElectromagnetics -  FDTD ]----------------*	[方程]: 麦克斯韦方程组 (旋度那两个)	[1] ▽ × H = ∂D/∂y + J		∂Hz/∂y - ∂Hy/∂z = ε∂Ex/∂t + σEx		∂Hx/∂z - ∂Hz/∂x = ε∂Ey/∂t + σEy		∂Hy/∂x - ∂Hx/∂y = ε∂Ez/∂t + σEz	[2] ▽ × E = - ∂B/∂y - Jm		∂Ez/∂y - ∂Ey/∂z =  - μ∂Hx/∂t - σHx		∂Ex/∂z - ∂Ez/∂x =  - μ∂Hy/∂t - σHy		∂Ey/∂x - ∂Ex/∂y =  - μ∂Hz/∂t - σHz**--------------------------------------*/struct ElecmagnCell{	double E[3] = { 0 }, H[3] = { 0 };			// 电场,磁场(三维)	double sigmaE = 0, sigmaM = 0;				// 电导率,磁导率	// J =σE  Jm =σm H 	double epsilon = 8.85E-12, mu = 1.2567E-6;	// ε介电常数,μ磁导系数 // D =εE  B =μH};void Electromagnetics(Tensor<ElecmagnCell>& Map, void (*setBoundaryValue) (Tensor<ElecmagnCell>& x, double time),	double deltaTime, double deltaDim[], int EndTimes) {	Tensor<ElecmagnCell> Mapprev(Map);	for (int time = 0; time < EndTimes; time++) {		for (int z = 1; z < Map.dimension[2] - 1; z++) {			for (int y = 1; y < Map.dimension[1] - 1; y++) {				for (int x = 1; x < Map.dimension[0] - 1; x++) {					int r[] = { x,y,z };					int rl[3][3] = { { x - 1,y,z },{ x,y - 1,z },{ x,y,z - 1 } };					int rr[3][3] = { { x + 1,y,z },{ x,y + 1,z },{ x,y,z + 1 } };					// CA,CB					double CA = 1 - Map(r).sigmaE * deltaTime / 2 / Map(r).epsilon;					double CB = deltaTime / Map(r).epsilon;					CA /= 1 + Map(r).sigmaE * deltaTime / 2 / Map(r).epsilon;					CB /= 1 + Map(r).sigmaE * deltaTime / 2 / Map(r).epsilon;					// CP,CQ					double CP = 1 - Map(r).sigmaM * deltaTime / 2 / Map(r).mu;					double CQ =  - deltaTime / Map(r).mu;					CP /= 1 + Map(r).sigmaM * deltaTime / 2 / Map(r).mu;					CQ /= 1 + Map(r).sigmaM * deltaTime / 2 / Map(r).mu;					for (int i = 0; i < 3; i++) {					//先算H 后算E, H要比E相差1/2						// H(t+1)	// ∂Ez/∂y - ∂Ey/∂z		∂Ex/∂z - ∂Ez/∂x		∂Ey/∂x - ∂Ex/∂y						Map(r).H[i] = CP * Map(r).H[i] + CQ * (							(Mapprev(rr[(i + 1) % 3]).E[(i + 2) % 3] - Mapprev(r).E[(i + 2) % 3]) / deltaDim[(i + 1) % 3]							- (Mapprev(rr[(i + 2) % 3]).E[(i + 1) % 3] - Mapprev(r).E[(i + 1) % 3]) / deltaDim[(i + 2) % 3]);					}					for (int i = 0; i < 3; i++) {						// E(t+1)	// ∂Hz/∂y - ∂Hy/∂z		∂Hx/∂z - ∂Hz/∂x		∂Hy/∂x - ∂Hx/∂y						Map(r).E[i] = CA * Map(r).E[i] + CB * (							(Mapprev(r).H[(i + 2) % 3] - Mapprev(rl[(i + 1) % 3]).H[(i + 2) % 3]) / deltaDim[(i + 1) % 3]							- (Mapprev(r).H[(i + 1) % 3] - Mapprev(rl[(i + 2) % 3]).H[(i + 1) % 3]) / deltaDim[(i + 2) % 3]);					}				}			}		}setBoundaryValue(Map, time* deltaTime);		Mapprev = Map;	}}int len = 40;void boundaryValue(Tensor<ElecmagnCell>& x, double time){	int p[] = { len / 2,len / 2,len / 2 };	int f = 1e7;	x(p).E[2] = 10 * sin(time * 2 * 3.1415926 * f);	printf("%f\n", 10 * sin(time * 2 * 3.1415926 * f));}int main(){	printf("5");	int size[] = { len,len,len };	double deltaDim[] = { 1,1,1 };	Tensor<ElecmagnCell> Map(3, size);	for (int i = 0; i < len * len * len; i++) { Map[i].epsilon = 8.85E-12, Map[i].mu = 1.2567E-6; }	Electromagnetics(Map, boundaryValue, 1e-9, deltaDim, 200);	FILE* fp = fopen("D:/LIGU.m", "w+");	fprintf(fp, "x = 1:40;y = 1:40;z = 1:40;[X, Y, Z] = meshgrid(x, y, z);U = X; V = Y; W = Z; \n");	for (int z = 0; z < Map.dimension[2]; z++) {		for (int y = 0; y < Map.dimension[1]; y++) {			for (int x = 0; x < Map.dimension[0]; x++) {				int p[] = { x,y,z };				fprintf(fp, "U(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).E[0]);				fprintf(fp, "V(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).E[1]);				fprintf(fp, "W(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).E[2]);			}		}	}	fprintf(fp, "figure;hold on;quiver3(X,Y,Z,U,V,W);\n");	for (int z = 0; z < Map.dimension[2]; z++) {		for (int y = 0; y < Map.dimension[1]; y++) {			for (int x = 0; x < Map.dimension[0]; x++) {				int p[] = { x,y,z };				fprintf(fp, "U(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).H[0]);				fprintf(fp, "V(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).H[1]);				fprintf(fp, "W(%d,%d,%d) = %f;\n", x + 1, y + 1, z + 1, Map(p).H[2]);			}		}	}	fprintf(fp, "quiver3(X,Y,Z,U,V,W,'r');\n");	fclose(fp);	Plot plot;	Mat<double> data(len, len);	for (int i = 0; i < len; i++) {		for (int j = 0; j < len; j++) {			int p[] = { i,j,len / 2 };			data[i * len + j] = Map(p).E[2];		}	}	plot.contourface(data, 5);	plot.g->PicWrite("D:/LIGU.ppm");}