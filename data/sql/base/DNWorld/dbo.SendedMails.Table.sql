USE [dnworld]
GO
/****** Object:  Table [dbo].[SendedMails]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[SendedMails](
	[CharacterID] [bigint] NOT NULL,
	[MailScheduleID] [int] NOT NULL,
	[SendDate] [datetime2](0) NOT NULL,
 CONSTRAINT [PK_SendedMails] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MailScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
