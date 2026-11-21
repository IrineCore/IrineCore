USE [dnworld]
GO
/****** Object:  Table [dbo].[AssignMissions]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AssignMissions](
	[CharacterID] [bigint] NOT NULL,
	[MissionTypeCode] [tinyint] NOT NULL,
	[AssignDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_AssignMissions] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MissionTypeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
