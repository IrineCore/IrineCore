USE [dnworld]
GO
/****** Object:  Table [dbo].[WorldEventQuestCounter]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[WorldEventQuestCounter](
	[EventQuestID] [int] NOT NULL,
	[Counter1] [int] NOT NULL,
 CONSTRAINT [PK_WorldEventQuestCounter] PRIMARY KEY CLUSTERED 
(
	[EventQuestID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
