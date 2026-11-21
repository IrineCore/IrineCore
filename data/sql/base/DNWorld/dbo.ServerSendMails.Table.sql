USE [dnworld]
GO
/****** Object:  Table [dbo].[ServerSendMails]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[ServerSendMails](
	[CharacterID] [bigint] NOT NULL,
	[MailID] [int] NOT NULL,
	[MailMemo] [nvarchar](30) COLLATE Chinese_PRC_CI_AS NULL,
 CONSTRAINT [PK_ServerSendMails] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[MailID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
