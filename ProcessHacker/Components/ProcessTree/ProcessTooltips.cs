﻿/*
 * Process Hacker
 * 
 * Copyright (C) 2008 wj32
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using System.Diagnostics;
using Aga.Controls.Tree;

namespace ProcessHacker
{
    public class ProcessTooltips : IToolTipProvider
    {
        private ProcessTree _tree;

        public ProcessTooltips(ProcessTree owner)
        {
            _tree = owner;
        }

        public string GetToolTip(TreeNodeAdv node, Aga.Controls.Tree.NodeControls.NodeControl nodeControl)
        {
            try
            {
                ProcessNode pNode = _tree.FindNode(node);

                string filename = "";

                if (pNode.PID == 4)
                {
                    filename = Misc.GetKernelFileName();
                }
                else
                {
                    filename = pNode.ProcessItem.Process.MainModule.FileName;
                }

                FileVersionInfo info = FileVersionInfo.GetVersionInfo(
                    Misc.GetRealPath(filename));

                string fileText = (pNode.ProcessItem.CmdLine != null ? (pNode.ProcessItem.CmdLine + "\n\n") : "") + info.FileName + "\n" +
                    info.FileDescription + " (" + info.FileVersion + ")\n" +
                    info.CompanyName;

                if (!Program.HackerWindow.ProcessServices.ContainsKey(pNode.PID))
                {
                    return fileText;
                }
                else
                {
                    string servicesText = "";

                    foreach (string service in Program.HackerWindow.ProcessServices[pNode.PID])
                    {
                        if (Program.HackerWindow.ServiceProvider.Dictionary.ContainsKey(service))
                        {
                            if (Program.HackerWindow.ServiceProvider.Dictionary[service].Status.DisplayName != "")
                                servicesText += service + " (" +
                                Program.HackerWindow.ServiceProvider.Dictionary[service].Status.DisplayName + ")\n";
                            else
                                servicesText += service + "\n";
                        }
                        else
                        {
                            servicesText += service + "\n";
                        }
                    }

                    return fileText + "\n\nServices:\n" + servicesText.TrimEnd('\n');
                }
            }
            catch
            { }

            return string.Empty;
        }
    }
}
