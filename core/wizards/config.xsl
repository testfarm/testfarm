<?xml version="1.0"?>

<xsl:stylesheet version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<!-- TestFarm System Configuration stylesheet

This file is part of TestFarm,
the Test Automation Tool for Embedded Software.
Please visit http://www.testfarm.org.

TestFarm is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TestFarm is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TestFarm.  If not, see <http://www.gnu.org/licenses/>.
-->

<xsl:include href="/opt/testfarm/lib/styles.xsl"/>

<xsl:template match="/">
  <html>

    <head>
      <title>TestFarm System Configuration</title>
      <xsl:call-template name="styles" />
    </head>

    <body style="font-family:helvetica,arial,sans-serif;">
      <h1>TestFarm System Configuration</h1>

      <h2>System Information</h2>
      <table class="system">
        <tr class="header">
          <td class="header">Info</td>
          <td class="header">Content</td>
        </tr>
        <tr class="content">
          <td class="content">System Description</td>
          <td class="content"><xsl:value-of select="/CONFIG/DESCRIPTION"/></td>
        </tr>
        <xsl:for-each select="/CONFIG/INFO">
          <tr class="content">
            <td class="content"><xsl:value-of select="@id"/></td>
            <td class="content"><xsl:value-of select="."/></td>
          </tr>
        </xsl:for-each>
      </table>

      <xsl:if test="/CONFIG/SERVICE">
        <h2>Services</h2>
        <table class="system">
          <tr class="header">
            <td class="header">Service</td>
            <td class="header">Command</td>
            <td class="header">Mode</td>
            <td class="header">Info</td>
          </tr>
          <xsl:for-each select="/CONFIG/SERVICE">
            <xsl:variable name="disable" select="translate(@mode, 'disable', 'DISABLE') = 'DISABLE'" />
            <xsl:variable name="style">
              <xsl:if test="$disable">
                <xsl:value-of select="'text-decoration: line-through;'"/>
              </xsl:if>
            </xsl:variable>
            <tr class="content">
              <td class="content" style="{$style}"><xsl:value-of select="@id"/></td>
              <td class="content"><xsl:value-of select="@cmd"/></td>
              <td class="content"><xsl:value-of select="@mode"/></td>
              <td class="content">
                <xsl:for-each select="INFO">
                  <xsl:value-of select="@id"/>:<xsl:value-of select="."/><br/>
                </xsl:for-each>
              </td>
            </tr>
          </xsl:for-each>
        </table>
      </xsl:if>

      <h2>Interfaces, Features and Actions</h2>
      <table class="system">
        <tr class="header">
          <td class="header">Interface</td>
          <td class="header">Type</td>
          <td class="header">Address</td>
          <td class="header">Mode</td>
          <td class="header">Info</td>
          <td class="header">Features</td>
        </tr>
        <xsl:for-each select="/CONFIG/INTERFACE">
          <xsl:variable name="disable0" select="translate(@mode, 'disable', 'DISABLE') = 'DISABLE'" />
          <xsl:variable name="style0">
            <xsl:if test="$disable0">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>
          <tr class="content">
            <td class="content" style="{$style0}"><xsl:value-of select="@id"/></td>
            <td class="content"><xsl:value-of select="@type"/></td>
            <td class="content"><xsl:value-of select="@addr"/></td>
            <td class="content"><xsl:value-of select="@mode"/></td>
            <td class="content">
              <xsl:for-each select="INFO">
                <xsl:value-of select="@id"/>:<xsl:value-of select="."/><br/>
              </xsl:for-each>
            </td>
            <td class="content" style="{$style0}">
              <xsl:for-each select="FEATURE">
                <xsl:variable name="disable1" select="(translate(@mode, 'disable', 'DISABLE') = 'DISABLE') or $disable0" />
                <xsl:variable name="style1">
                  <xsl:if test="$disable1">
                    <xsl:value-of select="'text-decoration: line-through;'"/>
                  </xsl:if>
                </xsl:variable>

                <span style="{$style1}"><xsl:value-of select="@id"/></span>&#160;
              </xsl:for-each>
            </td>
          </tr>
        </xsl:for-each>
      </table>

      <br/>

      <table class="system">
        <tr class="header">
          <td class="header">Feature</td>
          <td class="header">Mode</td>
          <td class="header">Description</td>
          <td class="header">Interface</td>
          <td class="header">Actions</td>
        </tr>
        <xsl:for-each select="/CONFIG/*/FEATURE">
        <xsl:sort select="@id"/>
          <xsl:variable name="disable0" select="translate(../@mode, 'disable', 'DISABLE') = 'DISABLE'" />
          <xsl:variable name="style0">
            <xsl:if test="$disable0">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>

          <xsl:variable name="disable1" select="(translate(@mode, 'disable', 'DISABLE') = 'DISABLE') or $disable0" />
          <xsl:variable name="style1">
            <xsl:if test="$disable1">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>

          <tr class="content">
            <td class="content" style="{$style1}"><xsl:value-of select="@id"/></td>
            <td class="content"><xsl:value-of select="@mode"/></td>
            <td class="content"><xsl:value-of select="DESCRIPTION"/></td>
            <td class="content" style="{$style0}"><xsl:value-of select="../@id"/></td>
            <td class="content">
              <xsl:for-each select="ACTION">
                <xsl:variable name="disable2" select="$disable1" />
                <xsl:variable name="style2">
                  <xsl:if test="$disable2">
                    <xsl:value-of select="'text-decoration: line-through;'"/>
                  </xsl:if>
                </xsl:variable>

                <span style="{$style2}"><xsl:value-of select="@id"/></span>&#160;
              </xsl:for-each>
            </td>
          </tr>
        </xsl:for-each>
      </table>
      <br/>

      <table class="system">
        <tr class="header">
          <td class="header">Action</td>
          <td class="header">Prototype</td>
          <td class="header">Feature</td>
          <td class="header">Interface</td>
        </tr>
        <xsl:for-each select="/CONFIG/*/*/ACTION">
        <xsl:sort select="@id"/>
          <xsl:variable name="disable0" select="translate(../../@mode, 'disable', 'DISABLE') = 'DISABLE'" />
          <xsl:variable name="style0">
            <xsl:if test="$disable0">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>

          <xsl:variable name="disable1" select="(translate(../@mode, 'disable', 'DISABLE') = 'DISABLE') or $disable0" />
          <xsl:variable name="style1">
            <xsl:if test="$disable1">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>

          <xsl:variable name="disable2" select="$disable1" />
          <xsl:variable name="style2">
            <xsl:if test="$disable2">
              <xsl:value-of select="'text-decoration: line-through;'"/>
            </xsl:if>
          </xsl:variable>

          <tr class="content">
            <td class="content" style="{$style2}"><xsl:value-of select="@id"/></td>
            <td class="content">
              <xsl:if test="@proto">
                (<xsl:value-of select="@proto"/>)
              </xsl:if>
            </td>
            <td class="content" style="{$style1}"><xsl:value-of select="../@id"/></td>
            <td class="content" style="{$style0}"><xsl:value-of select="../../@id"/></td>
          </tr>
        </xsl:for-each>
      </table>
    </body>

  </html>
</xsl:template>

</xsl:stylesheet>
