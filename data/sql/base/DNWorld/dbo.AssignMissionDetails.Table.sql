USE [dnworld]
GO
/****** Object:  Table [dbo].[AssignMissionDetails]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AssignMissionDetails](
	[CharacterID] [bigint] NOT NULL,
	[MissionTypeCode] [tinyint] NOT NULL,
	[MissionIndex] [tinyint] NOT NULL,
	[MissionID] [int] NOT NULL,
	[MissionCounter] [int] NOT NULL,
	[AchieveFlag] [bit] NOT NULL,
 CONSTRAINT [PK_AssignMissionDetails] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MissionTypeCode] ASC,
	[MissionIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[AssignMissionDetails]  WITH CHECK ADD  CONSTRAINT [FK_AssignMissions_AssignMissionDetails_CharacterID_MissionTypeCode] FOREIGN KEY([CharacterID], [MissionTypeCode])
REFERENCES [dbo].[AssignMissions] ([CharacterID], [MissionTypeCode])
GO
ALTER TABLE [dbo].[AssignMissionDetails] CHECK CONSTRAINT [FK_AssignMissions_AssignMissionDetails_CharacterID_MissionTypeCode]
GO
