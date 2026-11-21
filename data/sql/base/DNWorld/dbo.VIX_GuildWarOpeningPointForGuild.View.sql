USE [dnworld]
GO
/****** Object:  View [dbo].[VIX_GuildWarOpeningPointForGuild]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE VIEW [dbo].[VIX_GuildWarOpeningPointForGuild] WITH SCHEMABINDING AS
                    SELECT GuildID, SUM(OpeningPoint) AS OpeningPoint, COUNT_BIG(*) AS Cnt
                    FROM dbo.GuildWarOpeningPointRecord
                    GROUP BY GuildID
GO
SET ARITHABORT ON
SET CONCAT_NULL_YIELDS_NULL ON
SET QUOTED_IDENTIFIER ON
SET ANSI_NULLS ON
SET ANSI_PADDING ON
SET ANSI_WARNINGS ON
SET NUMERIC_ROUNDABORT OFF
GO
/****** Object:  Index [IX_UC_VIX_GuildWarOpeningPointForGuild]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE UNIQUE CLUSTERED INDEX [IX_UC_VIX_GuildWarOpeningPointForGuild] ON [dbo].[VIX_GuildWarOpeningPointForGuild]
(
	[GuildID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
