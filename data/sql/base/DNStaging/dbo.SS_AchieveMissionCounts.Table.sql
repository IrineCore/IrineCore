USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_AchieveMissionCounts]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_AchieveMissionCounts](
	[CharacterID] [bigint] NOT NULL,
	[MissionCount] [smallint] NOT NULL,
	[MissionScore] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_AchieveMissionCounts]    Script Date: 11/14/2026 9:00:09 AM ******/
CREATE CLUSTERED INDEX [IX_NC_AchieveMissionCounts] ON [dbo].[SS_AchieveMissionCounts]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
