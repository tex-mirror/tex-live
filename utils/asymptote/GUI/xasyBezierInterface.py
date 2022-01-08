#!/usr/bin/env python3


import PyQt5.QtWidgets as QtWidgets
import PyQt5.QtCore as QtCore
import PyQt5.QtGui as QtGui
import xasy2asy as xasy2asy
import xasyUtils as xasyUtils
import Widg_editBezier as Web

import InplaceAddObj
import math

class CurrentlySelctedType:
    none = -1
    node = 0
    ctrlPoint = 1

class InteractiveBezierEditor(InplaceAddObj.InplaceObjProcess):
    editAccepted = QtCore.pyqtSignal()
    editRejected = QtCore.pyqtSignal()

    def __init__(self, parent: QtCore.QObject, obj: xasy2asy.xasyDrawnItem, info: dict={}):
        super().__init__(parent)
        self.info = info
        self.asyPathBackup = xasy2asy.asyPath.fromPath(obj.path)
        self.asyPath = obj.path
        self.curveMode = self.asyPath.containsCurve
        assert isinstance(self.asyPath, xasy2asy.asyPath)
        self.transf = obj.transfKeymap[obj.transfKey][0]
        self._active = True

        self.currentSelMode = None
        # (Node index, Node subindex for )
        self.currentSelIndex = (None, 0)

        self.nodeSelRects = []
        self.ctrlSelRects = []

        self.setSelectionBoundaries()

        self.lastSelPoint = None
        self.preCtrlOffset = None
        self.postCtrlOffset = None
        self.inTransformMode = False

        self.opt = None
        self.obj = obj

        self.prosectiveNodes = []
        self.prospectiveCtrlPts = []

        #The magnification isn't being set. Here I'm manually setting it to be the square root of the determinant.
        self.info['magnification'] = math.sqrt(self.transf.xx * self.transf.yy - self.transf.xy * self.transf.yx)
        self.parent = parent
        if isinstance(obj,xasy2asy.xasyFilledShape) or isinstance(obj,xasy2asy.xasyShape):
            parent.ui.btnFill.setChecked(obj.path.fill)

    def swapObjFill(self):
        self.obj.swapFill() #This may end up being more in the future

    def setSelectionBoundaries(self):
        self.nodeSelRects = self.handleNodeSelectionBounds()

        if self.curveMode:
            self.ctrlSelRects = self.handleCtrlSelectionBoundaries()

    def handleNodeSelectionBounds(self):
        nodeSelectionBoundaries = []

        for node in self.asyPath.nodeSet:
            if node == 'cycle':
                nodeSelectionBoundaries.append(None)
                continue

            selEpsilon = 6/self.info['magnification']
            newRect = QtCore.QRect(0, 0, 2 * selEpsilon, 2 * selEpsilon)
            x, y = self.transf * node
            x = int(round(x))
            y = int(round(y))
            newRect.moveCenter(QtCore.QPoint(x, y))

            nodeSelectionBoundaries.append(newRect)

        return nodeSelectionBoundaries

    def handleCtrlSelectionBoundaries(self):
        ctrlPointSelBoundaries = []

        for nodes in self.asyPath.controlSet:
            nodea, nodeb = nodes

            selEpsilon = 6/self.info['magnification']

            newRect = QtCore.QRect(0, 0, 2 * selEpsilon, 2 * selEpsilon)
            newRectb = QtCore.QRect(0, 0, 2 * selEpsilon, 2 * selEpsilon)

            x, y = self.transf * nodea
            x2, y2 = self.transf * nodeb

            x = int(round(x))
            y = int(round(y))

            x2 = int(round(x2))
            y2 = int(round(y2))

            newRect.moveCenter(QtCore.QPoint(x, y))
            newRectb.moveCenter(QtCore.QPoint(x2, y2))

            ctrlPointSelBoundaries.append((newRect, newRectb))

        return ctrlPointSelBoundaries


    def postDrawPreview(self, canvas: QtGui.QPainter):
        assert canvas.isActive()

        dashedPen = QtGui.QPen(QtCore.Qt.DashLine)
        dashedPen.setCosmetic(True)
        # draw the base points
        canvas.save()
        canvas.setWorldTransform(self.transf.toQTransform(), True)

        epsilonSize = 6/self.info['magnification']

        if self.info['autoRecompute'] or not self.curveMode:
            ctrlPtsColor = 'gray'
        else:
            ctrlPtsColor = 'red'

        canvas.setPen(dashedPen)

        canvas.drawPath(self.asyPath.toQPainterPath())

        nodePen = QtGui.QPen(QtGui.QColor('blue'))
        nodePen.setCosmetic(True)

        ctlPtsPen = QtGui.QPen(QtGui.QColor(ctrlPtsColor))
        ctlPtsPen.setCosmetic(True)

        for index in range(len(self.asyPath.nodeSet)):
            point = self.asyPath.nodeSet[index]

            if point != 'cycle':
                basePoint = QtCore.QPointF(point[0], point[1])
                canvas.setPen(nodePen)
                canvas.drawEllipse(basePoint, epsilonSize, epsilonSize)
            else:
                point = self.asyPath.nodeSet[0]
                basePoint = QtCore.QPointF(point[0], point[1])
            if self.curveMode:
                if index != 0:
                    canvas.setPen(ctlPtsPen)
                    postCtrolSet = self.asyPath.controlSet[index - 1][1]
                    postCtrlPoint = QtCore.QPointF(postCtrolSet[0], postCtrolSet[1])
                    canvas.drawEllipse(postCtrlPoint, epsilonSize, epsilonSize)

                    canvas.setPen(dashedPen)
                    canvas.drawLine(basePoint, postCtrlPoint)

                if index != len(self.asyPath.nodeSet) - 1:
                    canvas.setPen(ctlPtsPen)
                    preCtrlSet = self.asyPath.controlSet[index][0]
                    preCtrlPoint = QtCore.QPointF(preCtrlSet[0], preCtrlSet[1])
                    canvas.drawEllipse(preCtrlPoint, epsilonSize, epsilonSize)

                    canvas.setPen(dashedPen)
                    canvas.drawLine(basePoint, preCtrlPoint)

        canvas.restore()

    def getPreAndPostCtrlPts(self, index):
        isCycle = self.asyPath.nodeSet[-1] == 'cycle'

        if index == 0 and not isCycle:
            preCtrl = None
        else:
            preCtrl = self.asyPath.controlSet[index - 1][1]

        if index == len(self.asyPath.nodeSet) - 1 and not isCycle:
            postCtrl = None
        else:
            postCtrl = self.asyPath.controlSet[index % (len(self.asyPath.nodeSet) - 1)][0]

        return preCtrl, postCtrl

    def findLinkingNode(self, index, subindex):
        """index and subindex are of the control points list."""
        if subindex == 0:
            return index
        else:
            if self.asyPath.nodeSet[index + 1] == 'cycle':
                return 0
            else:
                return index + 1

    def resetObj(self):
        self.asyPath.setInfo(self.asyPathBackup)
        self.setSelectionBoundaries()

    def mouseDown(self, pos, info, mouseEvent: QtGui.QMouseEvent=None):
        self.lastSelPoint = pos
        if self.inTransformMode:
            return

        if self.prosectiveNodes and not self.inTransformMode:
            self.currentSelMode = CurrentlySelctedType.node
            self.currentSelIndex = (self.prosectiveNodes[0], 0)
            self.inTransformMode = True
            self.parentNodeIndex = self.currentSelIndex[0]
        elif self.prospectiveCtrlPts and not self.inTransformMode:
            self.currentSelMode = CurrentlySelctedType.ctrlPoint
            self.currentSelIndex = self.prospectiveCtrlPts[0]
            self.inTransformMode = True
            self.parentNodeIndex = self.findLinkingNode(*self.currentSelIndex)

        if self.inTransformMode:
            parentNode = self.asyPath.nodeSet[self.parentNodeIndex]

            # find the offset of each control point to the node
            if not self.curveMode:
                return

            preCtrl, postCtrl = self.getPreAndPostCtrlPts(self.parentNodeIndex)

            if parentNode == 'cycle':
                parentNode = self.asyPath.nodeSet[0]
                self.parentNodeIndex = 0

            if preCtrl is not None:
                self.preCtrlOffset = xasyUtils.funcOnList(
                    preCtrl, parentNode, lambda a, b: a - b)
            else:
                self.preCtrlOffset = None

            if postCtrl is not None:
                self.postCtrlOffset = xasyUtils.funcOnList(
                    postCtrl, parentNode, lambda a, b: a - b)
            else:
                self.postCtrlOffset = None

    def mouseMove(self, pos, event: QtGui.QMouseEvent):
        if self.currentSelMode is None and not self.inTransformMode:
            # in this case, search for prosective nodes.
            prospectiveNodes = []
            prospectiveCtrlpts = []

            for i in range(len(self.nodeSelRects)):
                rect = self.nodeSelRects[i]
                if rect is None:
                    continue
                if rect.contains(pos):
                    prospectiveNodes.append(i)

            self.prosectiveNodes = prospectiveNodes

            if not self.info['autoRecompute'] and self.curveMode:
                for i in range(len(self.ctrlSelRects)):
                    recta, rectb = self.ctrlSelRects[i]

                    if recta.contains(pos):
                        prospectiveCtrlpts.append((i, 0))

                    if rectb.contains(pos):
                        prospectiveCtrlpts.append((i, 1))

                self.prospectiveCtrlPts = prospectiveCtrlpts
            else:
                self.prospectiveCtrlPts = []


        if self.inTransformMode:
            index, subindex = self.currentSelIndex
            newNode = (self.transf.inverted().toQTransform().map(pos.x(), pos.y()))
            if self.currentSelMode == CurrentlySelctedType.node:
                # static throughout the moving
                if self.asyPath.nodeSet[index] == 'cycle':
                    return

                self.asyPath.setNode(index, newNode)
                # if also move node:

                if self.curveMode:
                    checkPre, checkPost = self.getPreAndPostCtrlPts(index)

                    if 1 == 1: # TODO: Replace this with an option to also move control pts.
                        if checkPre is not None:
                            self.asyPath.controlSet[index - 1][1] = xasyUtils.funcOnList(
                                newNode, self.preCtrlOffset, lambda a, b: a + b
                            )
                        if checkPost is not None:
                            self.asyPath.controlSet[index][0] = xasyUtils.funcOnList(
                                newNode, self.postCtrlOffset, lambda a, b: a + b
                            )

                    if self.info['autoRecompute']:
                        self.quickRecalculateCtrls()


            elif self.currentSelMode == CurrentlySelctedType.ctrlPoint and self.curveMode:
                self.asyPath.controlSet[index][subindex] = newNode
                parentNode = self.asyPath.nodeSet[self.parentNodeIndex]

                if parentNode == 'cycle':
                    parentNode = self.asyPath.nodeSet[0]
                    isCycle = True
                else:
                    isCycle = False

                if self.parentNodeIndex == 0 and self.asyPath.nodeSet[-1] == 'cycle':
                    isCycle = True

                rawNewNode = xasyUtils.funcOnList(newNode, parentNode, lambda a, b: a - b)
                rawAngle = math.atan2(rawNewNode[1], rawNewNode[0])
                newNorm = xasyUtils.twonorm(rawNewNode)


                if self.info['editBezierlockMode'] >= Web.LockMode.angleLock:
                    otherIndex = 1 - subindex       # 1 if 0, 0 otherwise.
                    if otherIndex == 0:
                        if index < (len(self.asyPath.controlSet) - 1) or isCycle:
                            newIndex = 0 if isCycle else index + 1

                            oldOtherCtrlPnt = xasyUtils.funcOnList(
                                self.asyPath.controlSet[newIndex][0], parentNode, lambda a, b: a - b)

                            if self.info['editBezierlockMode'] >= Web.LockMode.angleAndScaleLock:
                                rawNorm = newNorm
                            else:
                                rawNorm = xasyUtils.twonorm(oldOtherCtrlPnt)

                            newPnt = (rawNorm * math.cos(rawAngle + math.pi),
                                rawNorm * math.sin(rawAngle + math.pi))

                            self.asyPath.controlSet[newIndex][0] = xasyUtils.funcOnList(
                                newPnt, parentNode, lambda a, b: a + b)
                    else:
                        if index > 0 or isCycle:
                            newIndex = -1 if isCycle else index - 1
                            oldOtherCtrlPnt = xasyUtils.funcOnList(
                                self.asyPath.controlSet[newIndex][1], parentNode, lambda a, b: a - b)

                            if self.info['editBezierlockMode'] >= Web.LockMode.angleAndScaleLock:
                                rawNorm = newNorm
                            else:
                                rawNorm = xasyUtils.twonorm(oldOtherCtrlPnt)

                            newPnt = (rawNorm * math.cos(rawAngle + math.pi),
                                      rawNorm * math.sin(rawAngle + math.pi))
                            self.asyPath.controlSet[newIndex][1] = xasyUtils.funcOnList(
                                newPnt, parentNode, lambda a, b: a + b)

    def recalculateCtrls(self):
        self.quickRecalculateCtrls()
        self.setSelectionBoundaries()

    def quickRecalculateCtrls(self):
        self.asyPath.controlSet.clear()
        self.asyPath.computeControls()

    def mouseRelease(self):
        if self.inTransformMode:
            self.inTransformMode = False
            self.currentSelMode = None

            self.setSelectionBoundaries()

    def forceFinalize(self):
        self.objectUpdated.emit()

    def createOptWidget(self, info):
        self.opt = Web.Widg_editBezier(self.info, self.curveMode)
        self.opt.ui.btnOk.clicked.connect(self.editAccepted)
        self.opt.ui.btnCancel.clicked.connect(self.editRejected)
        self.opt.ui.btnForceRecompute.clicked.connect(self.recalculateCtrls)

        return self.opt

    def getObject(self):
        pass

    def getXasyObject(self):
        pass
