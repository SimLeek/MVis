#include <freetype2/ft2build.h>
#include <freetype2/freetype/freetype.h>

#include <vtkVersion.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkChartXY.h>
#include <vtkTable.h>
#include <vtkPlot.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkPen.h>

#include "MathFunctions.hpp"

#include <boost/any.hpp>
#include <boost/variant.hpp>

#include <type_traits>

//todo: make the function plotter a subclass of this class and make this class only be a parent.
namespace mvis{
    template <class Number>
    class Graph{
    public:
        Graph(const char* x_axis_label = "X Axis"):
                table(vtkSmartPointer<vtkTable>::New()),
                x_axis(vtkSmartPointer<vtkFloatArray>::New()),
                view(vtkSmartPointer<vtkContextView>::New()),
                chart(vtkSmartPointer<vtkChartXY>::New())
        {
            x_axis->SetName(x_axis_label);
            table->AddColumn(x_axis);

            bg_color[0] = 1.0;
            bg_color[1] = 0.8;
            bg_color[2] = 0.3;

        }

        typedef std::function< Number(Number& x)>function_type;

        typedef std::pair<
                vtkSmartPointer<vtkFloatArray>,
                function_type
        > function_pair;

        typedef std::vector<function_pair> function_vector;

        void addFunction(const char* function_name,
                         function_type function){
            vtkSmartPointer<vtkFloatArray> column = vtkSmartPointer<vtkFloatArray>::New();
            column->SetName(function_name);
            table->AddColumn(column);
            function_list.push_back(
                    function_pair(column, function)//pass in function by value for speed
            );
            //todo: allow changing line color and theme
            line_design.push_back(255);
            line_design.push_back(10*line_design.size());
            line_design.push_back(2*line_design.size());
            line_design.push_back(255);
            line_design.push_back(1.0);
        }

        //todo: go back to single function method and have 2 different vectors(for single and multi) later
        //todo: instead of 0-10, it's going from 0-52
        void calcRegion(const size_t& x_min, const size_t& x_max, const Number& x_increment){
            if(x_min>x_max) return;

            size_t num_points = (Number)(x_max - x_min) / x_increment;
            table->SetNumberOfRows(num_points);
            for(int i=0; i<num_points;++i){
                Number x_value = (Number)i*x_increment + x_min;
                table->SetValue(i, 0, x_value); // set x axis
                for(int j = 0; j < function_list.size(); ++j){
                    Number f_value = function_list[j].second(x_value);
                    table->SetValue(i, j+1, f_value);
                }
            }
        }

        void setupView(){
            view->GetRenderer()->SetBackground(bg_color[0],bg_color[1],bg_color[2]);
            view->GetScene()->AddItem(chart);

            //chart->GetAxis(vtkAxis::BOTTOM)->GetLabelProperties()->SetColor(1.0,1.0,1.0);

            for(int i=0; i<function_list.size();++i) {
                vtkPlot *line = chart->AddPlot(vtkChart::LINE);


                //todo: for loop all the functions, not just first one
#if VTK_MAJOR_VERSION <= 5
                line->SetInput(table, 0, i+1);
#else
                line->SetInputData(table, 0, i+1);
#endif

                line->SetColor(line_design[i*5+0], line_design[i*5+1], line_design[i*5+2], line_design[i*5+3]);
                line->SetWidth(line_design[i*5+4]);
                line = chart->AddPlot(vtkChart::LINE);
            }

        }

        void showView(){
            view->GetInteractor()->Initialize();
            view->GetInteractor()->Start();
        }

    private:
        vtkSmartPointer<vtkTable> table;
        vtkSmartPointer<vtkFloatArray> x_axis;
    private:

        function_vector function_list;

        vtkSmartPointer<vtkContextView> view;
        float bg_color[3];



        vtkSmartPointer<vtkChartXY> chart;
        std::vector<float> line_design;
    };
}

int main() {

    mvis::Graph<float> graph;

    graph.addFunction("fib", mvis::fib<float>);

    graph.calcRegion(0, 10, 0.1);

    graph.setupView();
    graph.showView();

    return 0;
}