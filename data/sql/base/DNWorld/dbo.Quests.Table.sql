USE [dnworld]
GO
/****** Object:  Table [dbo].[Quests]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [dbo].[Quests](
	[CharacterID] [bigint] NOT NULL,
	[QuestIndex] [tinyint] NOT NULL,
	[QuestID] [int] NOT NULL,
	[QuestStatusCode] [tinyint] NOT NULL,
	[QuestStep] [smallint] NOT NULL,
	[QuestJournal] [tinyint] NOT NULL,
	[QuestExtra] [binary](90) NOT NULL,
	[Memo1] [tinyint] NOT NULL,
	[Memo2] [tinyint] NOT NULL,
	[Memo3] [tinyint] NOT NULL,
	[Memo4] [tinyint] NOT NULL,
	[Memo5] [tinyint] NOT NULL,
	[Memo6] [tinyint] NOT NULL,
	[Memo7] [tinyint] NOT NULL,
	[Memo8] [tinyint] NOT NULL,
	[Memo9] [tinyint] NOT NULL,
	[Memo10] [tinyint] NOT NULL,
	[QuestCode] [tinyint] NULL,
	[RegisterDate] [datetime2](0) NULL,
 CONSTRAINT [PK_Quests] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[QuestIndex] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
SET ANSI_PADDING OFF
GO
