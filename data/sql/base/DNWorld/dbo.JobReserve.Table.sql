USE [dnworld]
GO
/****** Object:  Table [dbo].[JobReserve]    Script Date: 11/14/2026 8:54:16 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[JobReserve](
	[SeqNo] [int] IDENTITY(1,1) NOT NULL,
	[Command] [nvarchar](max) COLLATE Chinese_PRC_CI_AS NOT NULL,
	[StatusCode] [tinyint] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
 CONSTRAINT [PK_JobReserve] PRIMARY KEY CLUSTERED 
(
	[SeqNo] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1] TEXTIMAGE_ON [DataFG1]
GO
/****** Object:  Index [IX_NN_JobReserve_StatusCode]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_JobReserve_StatusCode] ON [dbo].[JobReserve]
(
	[StatusCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
