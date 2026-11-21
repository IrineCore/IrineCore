USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCTeamMembers]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCTeamMembers](
	[DWCTeamID] [int] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[TeamLeaderFlag] [bit] NOT NULL,
	[RegisterDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_DWCTeamMembers] PRIMARY KEY CLUSTERED 
(
	[DWCTeamID] ASC,
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UN_DWCTeamMembers_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UN_DWCTeamMembers_CharacterID] ON [dbo].[DWCTeamMembers]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
ALTER TABLE [dbo].[DWCTeamMembers]  WITH CHECK ADD  CONSTRAINT [FK_DWCTeams_DWCTeamMembers] FOREIGN KEY([DWCTeamID])
REFERENCES [dbo].[DWCTeams] ([DWCTeamID])
GO
ALTER TABLE [dbo].[DWCTeamMembers] CHECK CONSTRAINT [FK_DWCTeams_DWCTeamMembers]
GO
