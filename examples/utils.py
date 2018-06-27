#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Mar 26 18:35:11 2018

@author: karolis
"""
import os
# import cv2

import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

# from rpy2.robjects import r, pandas2ri
# pandas2ri.activate()


def plot_front(
    x,
    y,
    xlabel: str=None,
    ylabel: str=None,
    figsize: tuple=None,
    linecolor: str='black',
    linewidth: int=1,
    ref_point: list=None,
    ideal_point: list=None,
    fillcolor: str='black',
    fillalpha: float=0.10,
):
    assert len(x) == len(y)

    if ref_point is not None:
        assert len(ref_point) == 2

    if ideal_point is not None:
        assert len(ideal_point) == 2

    fig, axes = plt.subplots()
    if figsize is not None:
        fig.set_size_inches(figsize[0], figsize[1])

    x = sorted(x)
    y = sorted(y)

    axes.plot(
        [x[0], x[0]],
        [y[0], y[1]],
        color=linecolor,
        linewidth=linewidth
    )

    axes.plot(
        [x[0], x[1]],
        [y[1], y[1]],
        color=linecolor,
        linewidth=linewidth
    )

    axes.plot(
        [x[0], x[1]],
        [y[0], y[1]],
        'o',
        color='black'
    )

    if ref_point is not None:
        axes.add_patch(
            Rectangle(
                (ref_point[0], y[0]),
                x[0] - ref_point[0],
                y[1] - y[0],
                facecolor=fillcolor,
                linewidth=0,
                alpha=fillalpha
        ))

    if ref_point is not None and ideal_point is not None:
        axes.plot(
            [ref_point[0], ideal_point[0]],
            [ideal_point[1], ideal_point[1]],
            '--', 
            color='black',
            linewidth=1.0
        )

        axes.plot(
            [ideal_point[0], ideal_point[0]],
            [ideal_point[1], ref_point[1]],
            '--', 
            color='black',
            linewidth=1.0
        )

    for i in range(2, len(y)):
        axes.plot(
            [x[i - 1], x[i - 1]],
            [y[i - 1], y[i]],
            color=linecolor,
            linewidth=linewidth
        )

        axes.plot(
            [x[i - 1], x[i]],
            [y[i], y[i]],
            color=linecolor,
            linewidth=linewidth
        )

        axes.plot(
            x[i],
            y[i],
            'o',
            color='black'
        )

        if ref_point is not None:
            axes.add_patch(
                Rectangle(
                    (ref_point[0], y[i - 1]),
                    (x[i - 1] - ref_point[0]),
                    (y[i] - y[i - 1]),
                    facecolor=fillcolor,
                    linewidth=0,
                    alpha=fillalpha
            ))

    axes.plot(
        x[-1],
        y[-1],
        'o',
        color='black'
    )

    if ref_point is not None:
        axes.add_patch(Rectangle(
            (ref_point[0], y[-1]),
            (x[-1] - ref_point[0]),
            (ref_point[1] - y[-1]),
            facecolor=fillcolor,
            linewidth=0,
            alpha=fillalpha
        ))

    if xlabel:
        axes.set_xlabel(xlabel)

    if ylabel:
        axes.set_ylabel(ylabel)

    if ref_point is not None:
        axes.plot(
            ref_point[0],
            ref_point[1],
            'o',
            color='red',
        )

    if ideal_point is not None:
        axes.plot(
            ideal_point[0],
            ideal_point[1],
            'o',
            color='green'
        )

    if ref_point is not None and ideal_point is not None:
        axes.plot(
            [ref_point[0], ref_point[0]],
            [ref_point[1], ideal_point[1]],
            '--', 
            color='black',
            linewidth=1.0
        )

        axes.plot(
            [ref_point[0], ideal_point[0]],
            [ref_point[1], ref_point[1]],
            '--', 
            color='black',
            linewidth=1.0
        )

        axes.plot(
            [x[0], ref_point[0]],
            [y[0], y[0]],
            '--', 
            color='black',
            linewidth=1.0
        )

        axes.plot(
            [x[-1], x[-1]],
            [y[-1], ref_point[1]],
            '--', 
            color='black',
            linewidth=1.0
        )

    
# def product_profiles(num, model):

#     model.schedules[num].kg_inventory.to_csv('tmp1.csv')
#     model.schedules[num].kg_waste.to_csv('tmp2.csv')
#     model.schedules[num].kg_backlog.to_csv('tmp3.csv')

#     if not os.path.exists('figures'):
#         os.mkdir('figures')

#     r(
#     '''
#         function() {
#             library(zoo)
            
#             colors <- c(
#                 rgb(146, 208, 80, max=255),
#                 rgb(179, 129, 217, max=255),
#                 rgb(196, 189, 151, max=255),
#                 rgb(255, 0, 0, max=255)
#             )
            
#             names(colors) <- c("A", "B", "C", "D")
            
#             kg_demand <- read.csv("data/kg_demand.csv")
#             kg_demand$date <- as.yearmon(as.Date(kg_demand$date, "%Y-%m-%d"))

#             kg_inventory_target <- read.csv("data/kg_inventory_target.csv")
#             kg_inventory_target$date <- as.yearmon(as.Date(kg_inventory_target$date, "%Y-%m-%d"))

#             inventory_levels <- read.csv("tmp1.csv")
#             inventory_levels$date <- as.yearmon(inventory_levels$date)
            
#             waste_levels <- read.csv("tmp2.csv")
#             waste_levels$date <- as.yearmon(waste_levels$date)
            
#             backlog_levels <- read.csv("tmp3.csv")
#             backlog_levels$date <- as.yearmon(backlog_levels$date)

#             jpeg("figures/inventory_waste_backlog_levels.jpg", width=16, height=8, units='in', res=720);

#             par(mfrow=c(2, 2))

#             for (i in 2:ncol(kg_demand)) {
#                 bar_xcoor <- barplot(
#                     t(kg_demand[, i]), 
#                     ylab="kg", 
#                     ylim=c(
#                         0, 
#                         max(
#                             max(kg_inventory_target[, i]), 
#                             max(inventory_levels[, i])  
#                         ) + 10
#                     ),
#                     names.arg=format(kg_demand$date, '%b %Y'),
#                     las=2,
#                     col=colors[i-1],
#                     legend=paste("Product ", colnames(kg_demand)[1:ncol(kg_demand)][i], " demand"),
#                     args.legend=list(
#                         x="topleft",
#                         bty="n"
#                     )
#                 )

#                 lines(
#                     xy.coords(
#                         bar_xcoor,
#                         kg_inventory_target[, i]
#                     ),
#                     type="b",
#                     col=colors[i-1],
#                 )

#                 lines(
#                     xy.coords(
#                         bar_xcoor,
#                         inventory_levels[, i]
#                     ),
#                     type="b",
#                     pch=2,
#                     col="black",
#                 )
                
#                 lines(
#                     xy.coords(
#                         bar_xcoor,
#                         waste_levels[, i]
#                     ),
#                     type="b",
#                     pch=3,
#                     col="black",
#                 )
                
#                 lines(
#                     xy.coords(
#                         bar_xcoor,
#                         backlog_levels[, i]
#                     ),
#                     type="b",
#                     pch=4,
#                     col="black",
#                 )

#                 legend(
#                     "topright", 
#                     c(
#                         paste("Product ", colnames(kg_demand)[1:ncol(kg_demand)][i], " inventory targets"),
#                         paste("Product ", colnames(kg_demand)[1:ncol(kg_demand)][i], " inventory"),
#                         paste("Product ", colnames(kg_demand)[1:ncol(kg_demand)][i], " waste"),
#                         paste("Product ", colnames(kg_demand)[1:ncol(kg_demand)][i], " backlog")
#                     ), 
#                     pch=c(1, 2, 3, 4),
#                     lty=c(1, 1, 1, 1),
#                     box.lty=0,
#                     col=c(colors[i-1], "black", "black", "black"), 
#                 )
#             }

#             dev.off();
#         }
#     '''
#     )()

#     os.remove('tmp1.csv')
#     os.remove('tmp2.csv')
#     os.remove('tmp3.csv')

#     img = cv2.cvtColor(
#         cv2.imread('figures/inventory_waste_backlog_levels.jpg'), 
#         cv2.COLOR_BGR2RGB
#     )
#     fig = plt.figure(figsize=(60, 60))

#     plt.imshow(img)
#     plt.axis('off')
#     plt.show()


def pareto_front_r(
    model,
    objective_x: str, 
    objective_y: str, 
    labels: list=None,
    mark_boundaries=False
):      
    obj_x = [schedule.objectives[objective_x] for schedule in model.schedules]
    obj_y = [schedule.objectives[objective_y] for schedule in model.schedules]
    
    pareto_front_r = r(
        '''
            function(obj_x, obj_y, xlabel, ylabel, mark_boundaries=False) {
                library(calibrate)

                jpeg("figures/pareto_front.jpg", width=8, height=8, units='in', res=720)

                plot(
                    xy.coords(
                        obj_x,
                        obj_y
                    ),
                    type = "b",
                    xlab = xlabel,
                    ylab = ylabel
                )

                if (mark_boundaries) {
                    obj_x <- unlist(obj_x)
                    obj_y <- unlist(obj_y)

                    textxy(
                        c(obj_x[1], tail(obj_x, 1)),
                        c(obj_y[1], tail(obj_y, 1)),
                        lab = c("X", "Y"),
                        cex = 1.0,
                        offset = -0.9
                    )
                }

                dev.off();
            }
        '''        
    )
    
    if labels is None:
        pareto_front_r(obj_x, obj_y, mark_boundaries=mark_boundaries)
    else:
        pareto_front_r(
            obj_x,
            obj_y,
            xlabel=labels[0],
            ylabel=labels[1],
            mark_boundaries=mark_boundaries
        )
        
    img = cv2.cvtColor(
            cv2.imread('figures/pareto_front.jpg'),
            cv2.COLOR_BGR2RGB
    )

    fig = plt.figure(figsize=(20, 20))
    plt.imshow(img)
    plt.axis('off')
    plt.show()