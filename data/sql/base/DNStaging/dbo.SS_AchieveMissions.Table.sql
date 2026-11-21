USE [dnstaging]
GO
/****** Object:  Table [dbo].[SS_AchieveMissions]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SS_AchieveMissions](
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL,
 CONSTRAINT [PK_SS_AchieveMissions] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MissionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON, FILLFACTOR = 100) ON [PRIMARY]
) ON [PRIMARY]
GO
