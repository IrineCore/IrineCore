USE [dnworld]
GO
/****** Object:  Table [dbo].[AchieveMissions]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AchieveMissions](
	[CharacterID] [bigint] NOT NULL,
	[MissionID] [int] NOT NULL,
	[AchieveDate] [datetime2](0) NOT NULL,
	[MissionTypeCode] [tinyint] NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_AchieveMissions_AchieveDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_AchieveMissions_AchieveDate] ON [dbo].[AchieveMissions]
(
	[AchieveDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [PK_AchieveMissions]    Script Date: 11/14/2026 8:54:37 AM ******/
ALTER TABLE [dbo].[AchieveMissions] ADD  CONSTRAINT [PK_AchieveMissions] PRIMARY KEY NONCLUSTERED 
(
	[CharacterID] ASC,
	[MissionID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNIF_AchieveMissions_CharacterID_MissionTypeCode] ON [dbo].[AchieveMissions]
(
	[CharacterID] ASC,
	[MissionTypeCode] ASC
)
INCLUDE([MissionID]) 
WHERE ([MissionTypeCode]>=(0) AND [MissionTypeCode]<=(1))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
