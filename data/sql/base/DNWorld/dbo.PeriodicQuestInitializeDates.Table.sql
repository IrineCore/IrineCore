USE [dnworld]
GO
/****** Object:  Table [dbo].[PeriodicQuestInitializeDates]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PeriodicQuestInitializeDates](
	[Period] [tinyint] NOT NULL,
	[InitializeDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_PeriodicQuestInitializeDates] PRIMARY KEY CLUSTERED 
(
	[Period] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
