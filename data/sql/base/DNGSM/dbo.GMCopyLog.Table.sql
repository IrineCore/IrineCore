USE [dngsm]
GO
/****** Object:  Table [dbo].[GMCopyLog]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMCopyLog](
	[GMCopyLogID] [int] IDENTITY(1,1) NOT NULL,
	[LogDate] [datetime] NULL,
	[SessionID] [bigint] NULL,
	[RestrictType] [nvarchar](50) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
	[InitCoin] [bigint] NULL,
	[CurrentCoin] [bigint] NULL,
	[CharacterLevel] [int] NULL,
	[BanAmount] [bigint] NULL,
	[IllegalAmount] [bigint] NULL,
	[IP] [int] NULL,
	[CharacterName] [nvarchar](30) COLLATE SQL_Latin1_General_CP1_CI_AS NULL,
 CONSTRAINT [PK_GMCopyLog] PRIMARY KEY CLUSTERED 
(
	[GMCopyLogID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
GO
