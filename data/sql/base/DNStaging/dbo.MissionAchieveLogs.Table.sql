USE [dnstaging]
GO
/****** Object:  Table [dbo].[MissionAchieveLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[MissionAchieveLogs](
	[AchieveDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL
) ON [PRIMARY]
GO
